//
// Created by Jonathan Richard on 2024-02-03.
//

#include "GraphicsPipeline.h"
#include "ShaderStage.h"
#include "VertexInputState.h"
#include "fmt/format.h"

namespace opengl {

GraphicsPipeline::GraphicsPipeline(Context& context, const GraphicsPipelineDesc& desc_) : desc(desc_), WithContext(context)
{
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
                const auto& location = i;
                if (location < 0)
                {
                    throw std::runtime_error(fmt::format("Invalid location for vertex attribute: {}", location));
                }
                bindingAttribLocations[bindingIndex].push_back(location);
            }
        }
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
