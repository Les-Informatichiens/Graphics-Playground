//
// Created by Jonathan Richard on 2024-02-03.
//

#include "GraphicsPipeline.h"
#include "ShaderStage.h"
#include "VertexInputState.h"

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


    // Create the program
    // not done yet...

    // Setup the vertex attributes
    const auto& vertexInput = dynamic_cast<const VertexInputState*>(desc.vertexInputState.get());
    if (vertexInput)
    {
        for (const auto& [binding, bindingAttribs] : vertexInput->getBufferAttribMap())
        {
            bindingAttribLocations.push_back(binding);
        }
    }

}

void GraphicsPipeline::bindVertexAttributes(size_t bufferIndex, size_t offset)
{

}

}// namespace opengl
