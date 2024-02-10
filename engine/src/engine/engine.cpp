//
// Created by Jonathan Richard on 2024-01-29.
//

#include "engine/engine.h"
#include "glm/glm.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>

engine::engine(IDevice& graphicsDevice) : device(graphicsDevice) {
}

void engine::update(float dt)
{
    //renderer.draw();
    std::cout << "Engine updated in " << dt << "ms" << std::endl;
}
template<typename T>
using Ref = std::shared_ptr<T>;

void engine::render() {

    this->createShaderStages();
    this->createOffscreenFramebuffer(800, 600);
    if (sampler == nullptr)
    {
        SamplerStateDesc samplerDesc = SamplerStateDesc::newLinear();
        samplerDesc.addressModeU = SamplerAddressMode::Clamp;
        samplerDesc.addressModeV = SamplerAddressMode::Clamp;
        samplerDesc.mipFilter = SamplerMipFilter::Nearest;
        samplerDesc.depthCompareEnabled = true;
        sampler = device.createSamplerState(samplerDesc);
    }

    VertexInputStateDesc vertexInputStateDesc = VertexInputStateDescBuilder()
                                        .beginBinding(0)
                                            .addVertexAttribute(VertexAttributeFormat::Float3, "inPosition", 0)
                                        .endBinding()
                                        .beginBinding(1)
                                            .addVertexAttribute(VertexAttributeFormat::Float3, "inColor", 1)
                                        .endBinding()
                                        .beginBinding(2)
                                            .addVertexAttribute(VertexAttributeFormat::Float3, "inTexCoord", 2)
                                        .endBinding()
                                        .build();
    Ref<IVertexInputState> vertexInputState = device.createVertexInputState(vertexInputStateDesc);

    Ref<IGraphicsPipeline> pipeline = device.createGraphicsPipeline({
            .shaderStages = shaderStages,
            .vertexInputState = vertexInputState,
            .fragmentUnitSamplerMap = {
                    {0, "testTex"},
                    {1, "testSample"}
            }
    });

    struct TestUniform {
        glm::vec4 color;
        bool isOffscreen;
    };

    float value = (std::sin(clock() / 1000.0f)+1.0f)/2.0f;
    // set uniform data
    auto uniformData = TestUniform{
            .color = {value, value, value, value},
            .isOffscreen = true
    };

    Ref<IBuffer> uniformBuffer = device.createBuffer({
            .type = BufferDesc::BufferTypeBits::Uniform,
            .data = &uniformData,
            .size = sizeof(TestUniform),
            .storage = ResourceStorage::Shared
    });

    struct Vertex {
        glm::vec3 Pos;
        glm::vec3 Color;
        glm::vec2 TexCoord;
    };

    // 4 vertices for a quad
    std::vector<decltype(Vertex::Pos)> positions = {
            {0.5f, 0.5f, 0.0f},
            {0.5f, -0.5f, 0.0f},
            {-0.5f, -0.5f, 0.0f},
            {-0.5f, 0.5f, 0.0f}
    };
    std::vector<decltype(Vertex::Color)> colors = {
            {1.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, 1.0f},
            {1.0f, 1.0f, 1.0f}
    };
    std::vector<uint32_t> indices = {
            0, 1, 2,
            0, 2, 3
    };


    Ref<IBuffer> vertexBuffer = device.createBuffer({
            .type = BufferDesc::BufferTypeBits::Vertex,
            .data = positions.data(),
            .size = static_cast<uint32_t>(positions.size() * sizeof(Vertex::Pos))
    });

    Ref<IBuffer> vertexBuffer2 = device.createBuffer({
            .type = BufferDesc::BufferTypeBits::Vertex,
            .data = colors.data(),
            .size = static_cast<uint32_t>(colors.size() * sizeof(Vertex::Color))
    });

    Ref<IBuffer> indexBuffer = device.createBuffer({
            .type = BufferDesc::BufferTypeBits::Index,
            .data = indices.data(),
            .size = static_cast<uint32_t>(indices.size() * sizeof(uint32_t))
    });

    // main renderpass vertex data
    // draw a quad that covers the entire screen
    std::vector<decltype(Vertex::Pos)> main_positions = {
            {1.0f, 1.0f, 0.0f},
            {1.0f, -1.0f, 0.0f},
            {-1.0f, -1.0f, 0.0f},
            {-1.0f, 1.0f, 0.0f}
    };
    std::vector<decltype(Vertex::TexCoord)> main_texCoords = {
            {1.0f, 1.0f},
            {1.0f, 0.0f},
            {0.0f, 0.0f},
            {0.0f, 1.0f}
    };

    Ref<IBuffer> main_vertexBuffer = device.createBuffer({
            .type = BufferDesc::BufferTypeBits::Vertex,
            .data = main_positions.data(),
            .size = static_cast<uint32_t>(main_positions.size() * sizeof(Vertex::Pos))
    });
    Ref<IBuffer> main_vertexBuffer2 = device.createBuffer({
            .type = BufferDesc::BufferTypeBits::Vertex,
            .data = main_texCoords.data(),
            .size = static_cast<uint32_t>(main_texCoords.size() * sizeof(Vertex::TexCoord))
    });


    Ref<ICommandPool> commandPool = device.createCommandPool({});
    Ref<ICommandBuffer> commandBuffer = commandPool->acquireCommandBuffer({});

    // begin offscreen render pass
    commandBuffer->beginRenderPass({
            .renderPass = {
                    .colorAttachments = {
                            RenderPassDesc::ColorAttachmentDesc{
                                    LoadAction::Clear,
                                    StoreAction::Store,
                                    Color{0.0f, 0.0f, 0.0f, 1.0f}
                            }
                    },
                    .depthAttachment = RenderPassDesc::DepthAttachmentDesc{
                            LoadAction::Clear,
                            StoreAction::Store,
                            0, 0,
                            1.0f
                    }
            },
            .framebuffer = fbOffscreen,
    });
    commandBuffer->bindGraphicsPipeline(pipeline);
    commandBuffer->bindBuffer(0, vertexBuffer, 0);
    commandBuffer->bindBuffer(1, vertexBuffer2, 0);
    commandBuffer->bindBuffer(2, main_vertexBuffer2, 0);


    Ref<ITexture> testSample;
    {
        int32_t texWidth = 0;
        int32_t texHeight = 0;
        int32_t channels = 0;
        uint8_t* pixels = stbi_load("Windows_curtains_diff.png", &texWidth, &texHeight, &channels, STBI_rgb_alpha);

        testSample = device.createTexture(TextureDesc::new2D(TextureFormat::RGBA_UNorm8, texWidth, texHeight, TextureDesc::TextureUsageBits::Sampled));
//        auto texWidth = testSample->getWidth();
//        auto texHeight = testSample->getHeight();
//        std::vector<uint32_t> pixels(texWidth * texHeight);
//        for (uint32_t y = 0; y != texHeight; y++) {
//            for (uint32_t x = 0; x != texWidth; x++) {
//                // create a XOR pattern
//                pixels[y * texWidth + x] = 0xFF000000 + ((x ^ y) << 16) + ((x ^ y) << 8) + (x ^ y);
//            }
//        }
        testSample->upload(pixels, TextureRangeDesc::new2D(0, 0, texWidth, texHeight));
        stbi_image_free(pixels);
    }

    commandBuffer->bindBuffer(0, uniformBuffer, 0);

    commandBuffer->bindTexture(1, BindTarget::BindTarget_Fragment, testSample);
    commandBuffer->bindSamplerState(1, BindTarget::BindTarget_Fragment, sampler);

    commandBuffer->drawIndexed(PrimitiveType::Triangle, indices.size(), IndexFormat::UInt32, *indexBuffer, 0);
    commandBuffer->endRenderPass();

    // update uniform data
    uniformData.isOffscreen = false;
    uniformBuffer->data(&uniformData, sizeof(TestUniform), 0);

    // begin main render pass
    commandBuffer->beginRenderPass({
            .renderPass = {
                    .colorAttachments = {
                            RenderPassDesc::ColorAttachmentDesc{
                                    LoadAction::Clear,
                                    StoreAction::DontCare,
                                    Color{0.0f, 0.0f, 0.0f, 1.0f}
                            }
                    },
                    .depthAttachment = RenderPassDesc::DepthAttachmentDesc{
                            LoadAction::Clear,
                            StoreAction::DontCare,
                            0, 0,
                            1.0f
                    }
            },
            .framebuffer = nullptr, // use default framebuffer
    });
    commandBuffer->bindGraphicsPipeline(pipeline);
    commandBuffer->bindBuffer(0, main_vertexBuffer, 0);
    commandBuffer->bindBuffer(2, main_vertexBuffer2, 0);
    commandBuffer->bindBuffer(0, uniformBuffer, 0);
    commandBuffer->bindTexture(0, BindTarget::BindTarget_Fragment, fbOffscreen->getColorAttachment(0));
    commandBuffer->bindSamplerState(0, BindTarget::BindTarget_Fragment, sampler);
    commandBuffer->drawIndexed(PrimitiveType::Triangle, indices.size(), IndexFormat::UInt32, *indexBuffer, 0);
    commandBuffer->endRenderPass();

    commandPool->submitCommandBuffer(commandBuffer);


    std::cout << "Engine rendered" << std::endl;
}

void engine::createOffscreenFramebuffer(uint32_t width, uint32_t height)
{
    if (fbOffscreen != nullptr)
    {
        return;
    }

    auto texUsageBits = TextureDesc::TextureUsageBits::Attachment | TextureDesc::TextureUsageBits::Sampled;

    auto colorAttachment = device.createTexture(TextureDesc::new2D(TextureFormat::RGBA_UNorm8, width, height, texUsageBits));
    auto texWidth = colorAttachment->getWidth();
    auto texHeight = colorAttachment->getHeight();
    std::vector<uint32_t> pixels(texWidth * texHeight);
    for (uint32_t y = 0; y != texHeight; y++) {
        for (uint32_t x = 0; x != texWidth; x++) {
            // create a XOR pattern
            pixels[y * texWidth + x] = 0xFF000000 + ((x ^ y) << 16) + ((x ^ y) << 8) + (x ^ y);
        }
    }
    colorAttachment->upload(pixels.data(), TextureRangeDesc::new2D(0, 0, texWidth, texHeight));

    auto depthAttachment = device.createTexture(TextureDesc::new2D(TextureFormat::Z_UNorm24, width, height, texUsageBits));
    fbOffscreen = device.createFramebuffer({
        .colorAttachments = {{0, FramebufferAttachmentDesc{colorAttachment, nullptr}}},
        .depthAttachment = {depthAttachment, nullptr},
        .stencilAttachment = {nullptr, nullptr}
    });
}

void engine::createShaderStages()
{
    if (vs == nullptr)
    {
        vs = device.createShaderModule({
                .type = ShaderModuleType::Vertex,
                .code = R"(#version 310 es
                    precision mediump float;
                    layout(location = 0) in vec3 inPosition;
                    layout(location = 1) in vec3 inColor;
                    layout(location = 2) in vec2 inTexCoord;

                    layout(location = 0) out vec3 fragColor;
                    layout(location = 1) out vec2 fragTexCoord;

                    layout(std140, binding = 0) uniform TEST {
                        vec4 color;
                        bool isOffscreen;
                    } test;

                    void main() {
                        fragColor = test.color.rgb * inColor;
                        fragTexCoord = inTexCoord;
                        gl_Position = vec4(inPosition, 1.0);
                    }
                )"
        });
    }

    if (fs == nullptr)
    {

        fs = device.createShaderModule({
                .type = ShaderModuleType::Fragment,
                .code = R"(#version 310 es
                precision mediump float;
                layout(location = 0) in vec3 fragColor;
                layout(location = 1) in vec2 fragTexCoord;

                layout(location = 0) out vec4 outColor;

                layout(std140, binding = 0) uniform TEST {
                    vec4 color;
                    bool isOffscreen;
                } test;

                layout(binding = 0) uniform sampler2D testTex;
                layout(binding = 1) uniform sampler2D testSample;

                void main() {
                    if (!test.isOffscreen) {
                        outColor = texture(testTex, fragTexCoord);
                    } else {
                        outColor = mix(texture(testSample, fragTexCoord), vec4(fragColor, 1.0), 0.5);//vec4(fragTexCoord, 0.0, 1.0);
                    }
                }
            )"
        });
    }

    if (shaderStages == nullptr)
    {
        shaderStages = device.createPipelineShaderStages(PipelineShaderStagesDesc::fromRenderModules(vs, fs));
    }
}