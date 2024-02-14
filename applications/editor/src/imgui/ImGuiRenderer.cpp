//
// Created by Jonathan Richard on 2024-02-09.
//

#include "ImGuiRenderer.h"
#include <iostream>

namespace imgui {

void ImGuiRenderer::initialize(IDevice& device, uint32_t width, uint32_t height)
{
    // Create buffers
    {
        vertexBuffer = device.createBuffer(BufferDesc{ .type = BufferDesc::BufferTypeBits::Vertex, .data = nullptr, .size = 0, .storage = ResourceStorage::Shared});
        indexBuffer = device.createBuffer(BufferDesc{ .type = BufferDesc::BufferTypeBits::Index, .data = nullptr, .size = 0, .storage = ResourceStorage::Shared});
        uniformBuffer = device.createBuffer(BufferDesc{ .type = BufferDesc::BufferTypeBits::Uniform, .data = nullptr, .size = 4*4*sizeof(float), .storage = ResourceStorage::Shared});
    }

    // Create font texture
    {
        unsigned char* pixels;
        int texWidth, texHeight;
        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->GetTexDataAsRGBA32(&pixels, &texWidth, &texHeight);
        fontTexture = device.createTexture(TextureDesc::new2D(TextureFormat::RGBA_UNorm8,
                                                              texWidth, texHeight,
                                                              TextureDesc::TextureUsageBits::Sampled));
        fontTexture->upload(pixels, TextureRangeDesc::new2D(0, 0, texWidth, texHeight));

        io.Fonts->TexID = (ImTextureID)fontTexture.get();
    }

    // Create sampler state
    {
        samplerState = device.createSamplerState(SamplerStateDesc::newLinear());
    }

    // Create pipeline
    {
        // Create shader modules
        auto vs = device.createShaderModule({
                .type = ShaderModuleType::Vertex,
                .code = R"(
                    #version 450
                    layout(location = 0) in vec2 aPos;
                    layout(location = 1) in vec2 aTex;
                    layout(location = 2) in vec4 aColor;
                    layout(location = 0) out vec2 vTex;
                    layout(location = 1) out vec4 vColor;
                    layout(std140, binding = 0) uniform UBO
                    {
                        mat4 projection;
                    };
                    void main()
                    {
                        vTex = aTex;
                        vColor = aColor;
                        gl_Position = projection * vec4(aPos.xy, 0.0, 1.0);
                    }
                )"
        });
        auto fs = device.createShaderModule({
                .type = ShaderModuleType::Fragment,
                .code = R"(
                    #version 450
                    layout(location = 0) in vec2 vTex;
                    layout(location = 1) in vec4 vColor;
                    layout(location = 0) out vec4 FragColor;
                    layout(binding = 1) uniform sampler2D tex;
                    void main()
                    {
                        FragColor = vColor * texture(tex, vTex.st);
                    }
                )"
        });
        auto shaderStages = device.createPipelineShaderStages(PipelineShaderStagesDesc::fromRenderModules(vs, fs));

        // Create vertex input state
        auto vertexInputStateDesc = VertexInputStateDescBuilder()
                                            .beginBinding(0)
                                            .addVertexAttribute(VertexAttributeFormat::Float2, "aPos", 0)
                                            .addVertexAttribute(VertexAttributeFormat::Float2, "aTex", 1)
                                            .addVertexAttribute(VertexAttributeFormat::UByte4Norm, "aColor", 2)
                                            .endBinding().build();
        auto vertexInputState = device.createVertexInputState(vertexInputStateDesc);

        // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
        ColorBlendAttachmentStateDesc colorBlendAttachment = {
                .blendEnabled = true,
                .srcColorBlendFactor = BlendFactor::SrcAlpha,
                .dstColorBlendFactor = BlendFactor::OneMinusSrcAlpha,
                .colorBlendOp = BlendOp::Add,
                .srcAlphaBlendFactor = BlendFactor::One,
                .dstAlphaBlendFactor = BlendFactor::OneMinusSrc1Alpha,
                .alphaBlendOp = BlendOp::Add,
                .colorWriteMask = ColorWriteMask::All
        };

        // Create pipeline
        pipeline = device.createGraphicsPipeline({
                .shaderStages = shaderStages,
                .vertexInputState = vertexInputState,
                .colorBlendAttachmentStates = {colorBlendAttachment},
                .rasterizationState = {
                        .cullMode = CullMode::None,
                        .frontFace = FrontFace::CounterClockwise,
                        .polygonFillMode = PolygonFillMode::Fill,
                },
                .fragmentUnitSamplerMap = {
                        {1, "tex"}
                },
        });
    }

    // Create depth stencil state
    {
        // no depth testing, no depth writing, no stencil testing
        depthStencilState = device.createDepthStencilState({
                .depth = {
                        .writeEnabled = false,
                },
        });
    }
}

void ImGuiRenderer::newFrame()
{
}

void ImGuiRenderer::setupRenderState(ImDrawData* drawData, std::shared_ptr<IGraphicsPipeline>& pipeline, ICommandBuffer& commandBuffer, uint32_t fbWidth, uint32_t fbHeight)
{
    // Bind pipeline
    {
        commandBuffer.bindGraphicsPipeline(pipeline);
    }

    // Bind vertex buffer
    {
        commandBuffer.bindBuffer(0, vertexBuffer, 0);
    }

    {
        commandBuffer.bindViewport(Viewport{.x = 0, .y = 0, .width = static_cast<float>(fbWidth), .height = static_cast<float>(fbHeight)});
    }

    // Bind uniform buffer
    {

        float L = drawData->DisplayPos.x;
        float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
        float T = drawData->DisplayPos.y;
        float B = drawData->DisplayPos.y + drawData->DisplaySize.y;
        const float ortho_projection[4][4] = {
                { 2.0f/(R-L),   0.0f,         0.0f,   0.0f },
                { 0.0f,         2.0f/(T-B),   0.0f,   0.0f },
                { 0.0f,         0.0f,        -1.0f,   0.0f },
                { (R+L)/(L-R),  (T+B)/(B-T),  0.0f,   1.0f },
        };

        uniformBuffer->data(ortho_projection, 4*4*sizeof(float), 0);

        commandBuffer.bindBuffer(0, uniformBuffer, 0);
    }

    // Bind sampler state
    {
        commandBuffer.bindSamplerState(1, BindTarget::BindTarget_Fragment, samplerState);
    }

    // Bind depth stencil state
    {
        commandBuffer.bindDepthStencilState(depthStencilState);
    }
}

void ImGuiRenderer::renderDrawData(ImDrawData* drawData, IDevice& device, ICommandBuffer& commandBuffer, std::shared_ptr<IGraphicsPipeline> pipeline_)
{
    int fbWidth = (int)(drawData->DisplaySize.x * drawData->FramebufferScale.x);
    int fbHeight = (int)(drawData->DisplaySize.y * drawData->FramebufferScale.y);
    if (fbWidth <= 0 || fbHeight <= 0)
        return;

    // Setup render state
    setupRenderState(drawData, pipeline, commandBuffer, fbWidth, fbHeight);

    Viewport viewport = {
            /*.x = */ 0.0,
            /*.y = */ 0.0,
            /*.width = */ (drawData->DisplaySize.x * drawData->FramebufferScale.x),
            /*.height = */ (drawData->DisplaySize.y * drawData->FramebufferScale.y),
    };

    ImVec2 clip_off = drawData->DisplayPos;         // (0,0) unless using multi-viewports
    ImVec2 clip_scale = drawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

    // Render command lists
    for (int n = 0; n < drawData->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = drawData->CmdLists[n];

        const size_t vertexBufferSize = cmd_list->VtxBuffer.Size * sizeof(ImDrawVert);
        const size_t indexBufferSize = cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx);

        // Update vertex and index buffer if needed
        if (vertexBuffer == nullptr || vertexBuffer->getSize() < vertexBufferSize)
        {
            vertexBuffer = device.createBuffer(BufferDesc{ .type = BufferDesc::BufferTypeBits::Vertex, .data = nullptr, .size = static_cast<uint32_t>(vertexBufferSize), .storage = ResourceStorage::Shared});
            commandBuffer.bindBuffer(0, vertexBuffer, 0);
        }
        if (indexBuffer == nullptr || indexBuffer->getSize() < indexBufferSize)
        {
            indexBuffer = device.createBuffer(BufferDesc{ .type = BufferDesc::BufferTypeBits::Index, .data = nullptr, .size = static_cast<uint32_t>(indexBufferSize), .storage = ResourceStorage::Shared});
        }

        // Upload vertex and index data
        vertexBuffer->data(cmd_list->VtxBuffer.Data, vertexBufferSize, 0);
        indexBuffer->data(cmd_list->IdxBuffer.Data, indexBufferSize, 0);

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != NULL)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                // Project scissor/clipping rectangles into framebuffer space
                const ImVec2 clipMin((pcmd->ClipRect.x - clip_off.x) * clip_scale.x,
                                     (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
                const ImVec2 clipMax((pcmd->ClipRect.z - clip_off.x) * clip_scale.x,
                                     (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);

                if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y) {
                    continue;
                }

                // OpenGL Y-axis goes up (Vulkan and Metal are good)
                // https://www.saschawillems.de/blog/2019/03/29/flipping-the-vulkan-viewport/
                const ScissorRect rect{uint32_t(clipMin.x),
                                            isOpenGL ? uint32_t(viewport.height - clipMax.y)
                                                     : uint32_t(clipMin.y),
                                            uint32_t(clipMax.x - clipMin.x),
                                            uint32_t(clipMax.y - clipMin.y)};
                commandBuffer.bindScissor(rect);

                // Bind texture
                if (pcmd->TextureId != nullptr)
                {
                    auto* texture = reinterpret_cast<ITexture*>(pcmd->TextureId);
                    auto res = textureMap.insert({pcmd->TextureId, texture->shared_from_this()});
                    commandBuffer.bindTexture(1, BindTarget::BindTarget_Fragment, res.first->second);
                }

                // Draw
                commandBuffer.drawIndexed(PrimitiveType::Triangle, pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? IndexFormat::UInt16 : IndexFormat::UInt32, *indexBuffer, pcmd->IdxOffset * sizeof(ImDrawIdx));
            }
        }
    }
}


}
