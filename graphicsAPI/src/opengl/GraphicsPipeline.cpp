//
// Created by Jonathan Richard on 2024-02-03.
//

#include "GraphicsPipeline.h"
#include "ShaderStage.h"
#include "VertexInputState.h"
#include "graphicsAPI/common/GraphicsCommandBuffer.h"

namespace opengl {

GraphicsPipeline::GraphicsPipeline(Context& context, const GraphicsPipelineDesc& desc_) : desc(desc_), WithContext(context)
{
    activeBindingAttribLocations.reserve(64);
    unitSamplerLocations.fill(-1);
    this->initialize();
}

void GraphicsPipeline::initialize()
{
    const auto& shaderStages = dynamic_cast<const PipelineShaderStages*>(desc.shaderStages.get());
    if (!shaderStages)
    {
        throw std::runtime_error("GraphicsPipelineDesc::shaderStages is required");
    }

    if (shaderStages->getProgram() == 0)
    {
        throw std::runtime_error("GraphicsPipelineDesc::shaderStages::program is required");
    }

    reflection = std::make_shared<GraphicsPipelineReflection>(getContext(), *shaderStages);

    // Setup the vertex attributes
    const auto& vertexInput = dynamic_cast<const VertexInputState*>(desc.vertexInputState.get());
    if (vertexInput)
    {
        for (const auto& [bindingIndex, bindingAttribs]: vertexInput->getBufferAttribMap())
        {
            for (int i = 0; i < bindingAttribs.size(); ++i)
            {
                const auto& location = bindingAttribs[i].location;
                if (location < 0)
                {
//                    throw std::runtime_error(fmt::format("Invalid location for vertex attribute: {}", location));
                }
                bindingAttribLocations[bindingIndex].push_back(location);
            }
        }
    }

    // Setup the texture units
    for (const auto& [unit, samplerName] : desc.fragmentUnitSamplerMap)
    {
        GLint loc = reflection->getLocation(samplerName);
        if (loc >= 0)
        {
            unitSamplerLocations[unit] = loc;
        }
        else
        {
            // log warning
            std::cout << "Warning: No sampler found with name: " << samplerName << std::endl;
        }
    }

    // Setup the texture units
    for (const auto& [unit, samplerName] : desc.vertexUnitSamplerMap)
    {
        GLint loc = reflection->getLocation(samplerName);
        if (loc >= 0)
        {
            unitSamplerLocations[unit] = loc;
        }
        else
        {
            // log warning
            std::cout << "Warning: No sampler found with name: " << samplerName << std::endl;
        }
    }

    // Setup the blend state
    if (!desc.colorBlendAttachmentStates.empty())
    {
        uint8_t colorWriteBits = desc.colorBlendAttachmentStates[0].colorWriteMask;
        colorMask[0] = (colorWriteBits & EnumToValue(ColorWriteMask::R)) != 0;
        colorMask[1] = (colorWriteBits & EnumToValue(ColorWriteMask::G)) != 0;
        colorMask[2] = (colorWriteBits & EnumToValue(ColorWriteMask::B)) != 0;
        colorMask[3] = (colorWriteBits & EnumToValue(ColorWriteMask::A)) != 0;
    }

    if (!desc.colorBlendAttachmentStates.empty() && desc.colorBlendAttachmentStates[0].blendEnabled)
    {
        blendEnabled = true;
        blendMode = {convertBlendOp(desc.colorBlendAttachmentStates[0].colorBlendOp),
                     convertBlendOp(desc.colorBlendAttachmentStates[0].alphaBlendOp),
                     convertBlendFactor(desc.colorBlendAttachmentStates[0].srcColorBlendFactor),
                     convertBlendFactor(desc.colorBlendAttachmentStates[0].dstColorBlendFactor),
                     convertBlendFactor(desc.colorBlendAttachmentStates[0].srcAlphaBlendFactor),
                     convertBlendFactor(desc.colorBlendAttachmentStates[0].dstAlphaBlendFactor)};
    }
    else
    {
        blendEnabled = false;
    }

    cullMode = desc.rasterizationState.cullMode;
    frontFace = desc.rasterizationState.frontFace;
    fillMode = desc.rasterizationState.polygonFillMode;
}

void GraphicsPipeline::bind()
{
    if (auto shaderStages = dynamic_cast<PipelineShaderStages*>(desc.shaderStages.get()))
    {
        shaderStages->bind();
    }

    getContext().colorMask(colorMask[0], colorMask[1], colorMask[2], colorMask[3]);
    if (!desc.colorBlendAttachmentStates.empty() && blendEnabled)
    {
        getContext().enable(GL_BLEND);
        getContext().blendEquationSeparate(blendMode.blendOpColor, blendMode.blendOpAlpha);
        getContext().blendFuncSeparate(blendMode.srcColor, blendMode.dstColor, blendMode.srcAlpha, blendMode.dstAlpha);
    }
    else
    {
        getContext().disable(GL_BLEND);
    }

    if (cullMode != CullMode::None)
    {
        getContext().enable(GL_CULL_FACE);
        getContext().cullFace(cullMode == CullMode::Back ? GL_BACK : GL_FRONT);
    }
    else
    {
        getContext().disable(GL_CULL_FACE);
    }

    getContext().frontFace(frontFace == FrontFace::Clockwise ? GL_CW : GL_CCW);

    getContext().polygonFillMode(fillMode == PolygonFillMode::Fill ? GL_FILL : GL_LINE);
}

void GraphicsPipeline::unbind()
{
    if (auto shaderStages = dynamic_cast<PipelineShaderStages*>(desc.shaderStages.get()))
    {
        shaderStages->unbind();
    }
}

void GraphicsPipeline::bindTextureUnit(const size_t unit, uint8_t bindTarget)
{
    if (!desc.shaderStages)
    {
        return;
    }

    if (unit >= MAX_TEXTURE_SAMPLERS)
    {
        return;
    }

    GLint samplerLocation = -1;
    if (bindTarget == BindTarget::BindTarget_Vertex)
    {
        samplerLocation = unitSamplerLocations[unit]; // todo figure out what to do with this?
    }
    else
    {
        samplerLocation = unitSamplerLocations[unit];
    }

    if (samplerLocation >= 0)
    {
        getContext().uniform1i(samplerLocation, static_cast<GLint>(unit));
        getContext().activeTexture(GL_TEXTURE0 + unit);
    }
    else
    {
        // log warning
        std::cout << "Warning: No sampler found for texture unit: " << unit << std::endl;
    }

}

int GraphicsPipeline::getTextureUnitLocation(const size_t unit, uint8_t bindTarget)
{
    if (unit >= MAX_TEXTURE_SAMPLERS)
    {
        return -1;
    }

    GLint samplerLocation = -1;
    if (bindTarget == BindTarget::BindTarget_Vertex)
    {
        samplerLocation = unitSamplerLocations[unit];
    }
    else
    {
        samplerLocation = unitSamplerLocations[unit];
    }

    return samplerLocation;
}

void GraphicsPipeline::bindVertexAttributes(size_t bufferIndex, size_t offset)
{
    const auto& vertexInput = dynamic_cast<const VertexInputState*>(desc.vertexInputState.get());
    if (vertexInput)
    {
        const auto& bindingAttribs = vertexInput->getBufferAttributes(bufferIndex);
        auto& locations = bindingAttribLocations[bufferIndex];
        for (int i = 0; i < bindingAttribs.size(); ++i)
        {
            const auto location = locations[i];
            if (location < 0)
            {
                continue;
            }
            activeBindingAttribLocations.push_back(location);

            const auto& attrib = bindingAttribs[i];
            getContext().enableVertexAttribArray(location);
            getContext().vertexAttribPointer(
                    location,
                    attrib.numComponents,
                    attrib.type,
                    attrib.normalized,
                    attrib.stride,
                    reinterpret_cast<const char*>(attrib.offset) + offset);
//            std::cout << "Enabling vertex attrib array: " << location << std::endl;
        }
    }
}

void GraphicsPipeline::unbindVertexAttributes()
{
    for (const auto& location : activeBindingAttribLocations)
    {
//        std::cout << "Disabling vertex attrib array: " << location << std::endl;
        getContext().disableVertexAttribArray(location);
    }
    activeBindingAttribLocations.clear();
}
GLenum GraphicsPipeline::convertBlendOp(BlendOp value) {
    // sets blending equation for both RGA and Alpha
    switch (value) {
        case BlendOp::Add:
            return GL_FUNC_ADD;
        case BlendOp::Subtract:
            return GL_FUNC_SUBTRACT;
        case BlendOp::ReverseSubtract:
            return GL_FUNC_REVERSE_SUBTRACT;
        case BlendOp::Min:
            return GL_MIN;
        case BlendOp::Max:
            return GL_MAX;
    }
    return GL_FUNC_ADD; // default for unsupported values
}
GLenum GraphicsPipeline::convertBlendFactor(BlendFactor value) {
    switch (value) {
        case BlendFactor::Zero:
            return GL_ZERO;
        case BlendFactor::One:
            return GL_ONE;
        case BlendFactor::SrcColor:
            return GL_SRC_COLOR;
        case BlendFactor::OneMinusSrcColor:
            return GL_ONE_MINUS_SRC_COLOR;
        case BlendFactor::DstColor:
            return GL_DST_COLOR;
        case BlendFactor::OneMinusDstColor:
            return GL_ONE_MINUS_DST_COLOR;
        case BlendFactor::SrcAlpha:
            return GL_SRC_ALPHA;
        case BlendFactor::OneMinusSrcAlpha:
            return GL_ONE_MINUS_SRC_ALPHA;
        case BlendFactor::DstAlpha:
            return GL_DST_ALPHA;
        case BlendFactor::OneMinusDstAlpha:
            return GL_ONE_MINUS_DST_ALPHA;
        case BlendFactor::ConstantColor:
            return GL_CONSTANT_COLOR;
        case BlendFactor::OneMinusConstantColor:
            return GL_ONE_MINUS_CONSTANT_COLOR;
        case BlendFactor::ConstantAlpha:
            return GL_CONSTANT_ALPHA;
        case BlendFactor::OneMinusConstantAlpha:
            return GL_ONE_MINUS_CONSTANT_ALPHA;
        case BlendFactor::SrcAlphaSaturate:
            return GL_SRC_ALPHA_SATURATE;
        case BlendFactor::Src1Color:
            return GL_ONE; // default for unsupported values
        case BlendFactor::OneMinusSrc1Color:
            return GL_ONE; // default for unsupported values
        case BlendFactor::Src1Alpha:
            return GL_ONE; // default for unsupported values
        case BlendFactor::OneMinusSrc1Alpha:
            return GL_ONE; // default for unsupported values
    }
    return GL_ONE; // default for unsupported values
}

}// namespace opengl
