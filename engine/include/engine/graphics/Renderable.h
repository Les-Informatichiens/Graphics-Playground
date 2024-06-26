//
// Created by Jonathan Richard on 2024-02-12.
//

#pragma once

#include "Material.h"
#include "VertexData.h"
#include "graphicsAPI/common/Device.h"
#include "graphicsAPI/common/GraphicsCommandBuffer.h"

namespace graphics {

class Renderable
{
public:
    Renderable(std::shared_ptr<Material> material, std::shared_ptr<VertexData> vertexData);

    void draw(IDevice& device, IGraphicsCommandBuffer& cmdBuffer) const;

    GraphicsPipelineDesc buildGraphicsPipelineDesc() const;
    void injectGraphicsPipeline(std::shared_ptr<IGraphicsPipeline> pipeline) { graphicsPipeline = std::move(pipeline); }

    [[nodiscard]] std::shared_ptr<Material> getMaterial() const { return material; }
    [[nodiscard]] std::shared_ptr<IVertexData> getVertexData() const { return vertexData; }

    [[nodiscard]] std::shared_ptr<IGraphicsPipeline> getGraphicsPipeline() const { return graphicsPipeline; }

    void setElementCount(uint32_t count) { elementCount = count; }
private:
    std::shared_ptr<Material> material;
    std::shared_ptr<IVertexData> vertexData;

    int elementCount = 0;

    std::shared_ptr<IGraphicsPipeline> graphicsPipeline;
};

} // namespace graphics