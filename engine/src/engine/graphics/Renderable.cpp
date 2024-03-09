//
// Created by Jonathan Richard on 2024-02-12.
//

#include "engine/graphics/Renderable.h"

namespace graphics {

Renderable::Renderable(std::shared_ptr<Material> material, std::shared_ptr<VertexData> vertexData)
    : material(std::move(material)), vertexData(std::move(vertexData))
{
}

//void Renderable::setVertexData(std::shared_ptr<IBuffer> buffer, std::shared_ptr<IBuffer> indexBuffer_, uint32_t elementCount_)
//{
//    vertexBuffer = std::move(buffer);
//    indexBuffer = std::move(indexBuffer_);
//    elementCount = elementCount_;
//}

void Renderable::draw(IDevice& device, ICommandBuffer& cmdBuffer) const
{
    cmdBuffer.bindGraphicsPipeline(graphicsPipeline);
    material->bind(device, cmdBuffer);
    cmdBuffer.bindBuffer(0, vertexData->getVertexBuffer(), 0);
    cmdBuffer.drawIndexed(PrimitiveType::Triangle, vertexData->getIndexCount(), vertexData->getIndexFormat(), *vertexData->getIndexBuffer(), 0);
}

GraphicsPipelineDesc Renderable::buildGraphicsPipelineDesc() const
{
    GraphicsPipelineDesc desc;
    vertexData->preparePipelineDesc(desc);
    material->preparePipelineDesc(desc);
    return desc;
}

//void Renderable::setElementCount(uint32_t count)
//{
//    elementCount = count;
//}

}// namespace graphics
