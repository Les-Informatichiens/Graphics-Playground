//
// Created by Jonathan Richard on 2024-03-06.
//

#include "engine/graphics/ShaderProgram.h"

namespace graphics {

ShaderProgram::ShaderProgram(IDevice& device, const std::shared_ptr<IShaderModule>& vertexShader, const std::shared_ptr<IShaderModule>& fragmentShader, const std::shared_ptr<IVertexInputState>& vertexInputState)
{
    initialize(device, vertexShader, fragmentShader, vertexInputState);
}

void ShaderProgram::initialize(IDevice& device, const std::shared_ptr<IShaderModule>& vertexShader, const std::shared_ptr<IShaderModule>& fragmentShader, const std::shared_ptr<IVertexInputState>& vertexInputState)
{
    shaderStages = device.createPipelineShaderStages(PipelineShaderStagesDesc::fromRenderModules(vertexShader, fragmentShader));
    pipelineDesc.shaderStages = shaderStages;
    pipelineDesc.vertexInputState = std::move(vertexInputState);
    cachedPipeline = device.createGraphicsPipeline(pipelineDesc);
}

}// namespace graphics
