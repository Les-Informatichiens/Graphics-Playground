//
// Created by Jonathan Richard on 2024-02-03.
//

#include "GraphicsPipeline.h"
#include "ShaderStage.h"
#include "VertexInputState.h"
#include "graphicsAPI/common/CommandBuffer.h"

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
    }

    // Setup the texture units
    for (const auto& [unit, samplerName] : desc.vertexUnitSamplerMap)
    {
        GLint loc = reflection->getLocation(samplerName);
        if (loc >= 0)
        {
            unitSamplerLocations[unit] = loc;
        }
    }

}

void GraphicsPipeline::bind()
{
    if (auto shaderStages = dynamic_cast<PipelineShaderStages*>(desc.shaderStages.get()))
    {
        shaderStages->bind();
    }
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
        }
    }
}

void GraphicsPipeline::unbindVertexAttributes()
{
    for (const auto& location : activeBindingAttribLocations)
    {
        getContext().disableVertexAttribArray(location);
    }
    activeBindingAttribLocations.clear();
}

}// namespace opengl
