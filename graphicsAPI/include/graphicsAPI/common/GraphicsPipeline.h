//
// Created by Jonathan Richard on 2024-02-01.
//

#pragma once

#include "ShaderStage.h"
#include "VertexInputState.h"
#include <memory>

struct GraphicsPipelineDesc
{
    std::shared_ptr<IPipelineShaderStages> shaderStages;
    std::shared_ptr<IVertexInputState> vertexInputState;
};

class IGraphicsPipeline
{
public:
    virtual ~IGraphicsPipeline() = default;

    [[nodiscard]] virtual const GraphicsPipelineDesc& getDesc() const = 0;
};