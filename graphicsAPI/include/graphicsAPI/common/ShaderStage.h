//
// Created by Jonathan Richard on 2024-02-01.
//

#pragma once

#include "ShaderModule.h"
#include <memory>
#include <utility>

enum class ShaderStagesType
{
    Graphics,
    Compute
};

struct PipelineShaderStagesDesc
{
    static PipelineShaderStagesDesc fromRenderModules(std::shared_ptr<IShaderModule> vertexModule, std::shared_ptr<IShaderModule> fragmentModule);
    static PipelineShaderStagesDesc fromComputeModule(std::shared_ptr<IShaderModule> computeModule);

    std::shared_ptr<IShaderModule> vertexModule;
    std::shared_ptr<IShaderModule> geometryModule;
    std::shared_ptr<IShaderModule> fragmentModule;
    std::shared_ptr<IShaderModule> computeModule;

    ShaderStagesType type = ShaderStagesType::Graphics;
};

class IPipelineShaderStages
{
public:
    virtual ~IPipelineShaderStages() = default;

    [[nodiscard]] virtual const std::shared_ptr<IShaderModule>& getVertexShader() const = 0;
    [[nodiscard]] virtual const std::shared_ptr<IShaderModule>& getFragmentShader() const = 0;
    [[nodiscard]] virtual const std::shared_ptr<IShaderModule>& getGeometryShader() const = 0;
    [[nodiscard]] virtual const std::shared_ptr<IShaderModule>& getComputeShader() const = 0;
};
