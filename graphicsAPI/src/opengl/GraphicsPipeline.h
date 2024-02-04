//
// Created by Jonathan Richard on 2024-02-03.
//

#pragma once

#include "graphicsAPI/common/GraphicsPipeline.h"
#include "graphicsAPI/opengl/Context.h"

namespace opengl
{

class GraphicsPipeline : public IGraphicsPipeline, public WithContext
{
public:
    GraphicsPipeline(Context& context, const GraphicsPipelineDesc& desc_);

    void initialize();

    void bindVertexAttributes(size_t bufferIndex, size_t offset);

    [[nodiscard]] const GraphicsPipelineDesc& getDesc() const override { return this->desc; }

private:
    GraphicsPipelineDesc desc;

    std::vector<uint32_t> bindingAttribLocations;
};

}
