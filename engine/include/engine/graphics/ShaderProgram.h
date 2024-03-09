//
// Created by Jonathan Richard on 2024-03-06.
//

#pragma once

#include "graphicsAPI/common/Device.h"
#include "graphicsAPI/common/ShaderModule.h"
#include "graphicsAPI/common/ShaderStage.h"
#include "graphicsAPI/common/GraphicsPipeline.h"

namespace graphics
{

class ShaderProgram
{
public:
    ShaderProgram(IDevice& device, const std::shared_ptr<IShaderModule>& vertexShader, const std::shared_ptr<IShaderModule>& fragmentShader, const std::shared_ptr<IVertexInputState>& vertexInputState);

//    std::shared_ptr<IGraphicsPipeline> getPipeline() { return cachedPipeline; };

    void preparePipelineDesc(GraphicsPipelineDesc& desc) const;
private:
    void initialize(IDevice& device, const std::shared_ptr<IShaderModule>& vertexShader, const std::shared_ptr<IShaderModule>& fragmentShader, const std::shared_ptr<IVertexInputState>& vertexInputState);

    std::shared_ptr<IPipelineShaderStages> shaderStages;
    GraphicsPipelineDesc pipelineDesc;
//    std::shared_ptr<IGraphicsPipeline> cachedPipeline;
};

} // namespace graphics