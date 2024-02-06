//
// Created by Jonathan Richard on 2024-02-03.
//

#pragma once

#include "GraphicsPipelineReflection.h"
#include "graphicsAPI/common/GraphicsPipeline.h"
#include "graphicsAPI/opengl/Context.h"

#include <map>

namespace opengl
{

class GraphicsPipeline : public IGraphicsPipeline, public WithContext
{
public:
    GraphicsPipeline(Context& context, const GraphicsPipelineDesc& desc_);

    void initialize();

    void bind();
    void unbind();

    void bindVertexAttributes(size_t bufferIndex, size_t offset);
    void unbindVertexAttributes();

    [[nodiscard]] const GraphicsPipelineReflection& getReflection() const { return *reflection; }
    [[nodiscard]] const GraphicsPipelineDesc& getDesc() const override { return this->desc; }

private:
    GraphicsPipelineDesc desc;

    std::map<uint32_t, std::vector<uint32_t>> bindingAttribLocations;
    std::vector<uint32_t> activeBindingAttribLocations;

//    std::vector<uint32_t> bindingAttribLocations;
    std::shared_ptr<GraphicsPipelineReflection> reflection;
};

}
