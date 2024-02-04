//
// Created by Jonathan Richard on 2024-02-03.
//

#include "graphicsAPI/common/ShaderStage.h"

PipelineShaderStagesDesc PipelineShaderStagesDesc::fromRenderModules(std::shared_ptr<IShaderModule> vertexModule, std::shared_ptr<IShaderModule> fragmentModule)
{
    PipelineShaderStagesDesc desc;
    desc.vertexModule = std::move(vertexModule);
    desc.fragmentModule = std::move(fragmentModule);
    desc.type = ShaderStagesType::Graphics;
    return desc;
}

PipelineShaderStagesDesc PipelineShaderStagesDesc::fromComputeModule(std::shared_ptr<IShaderModule> computeModule)
{
    PipelineShaderStagesDesc desc;
    desc.computeModule = std::move(computeModule);
    desc.type = ShaderStagesType::Compute;
    return desc;
}
