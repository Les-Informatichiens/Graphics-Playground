//
// Created by Jonathan Richard on 2024-02-12.
//

#include "engine/graphics/Renderable.h"

namespace graphics {

Renderable::Renderable(std::shared_ptr<Material> material)
    : material(std::move(material))
{
}

void Renderable::setVertexData(std::shared_ptr<IBuffer> buffer, std::shared_ptr<IBuffer> indexBuffer_, uint32_t elementCount_)
{
    vertexBuffer = std::move(buffer);
    indexBuffer = std::move(indexBuffer_);
    elementCount = elementCount_;
}

void Renderable::draw(IDevice& device, ICommandBuffer& cmdBuffer) const
{
    material->bind(device, cmdBuffer);
    cmdBuffer.bindBuffer(0, vertexBuffer, 0);
    cmdBuffer.drawIndexed(PrimitiveType::Triangle, elementCount, IndexFormat::UInt32, *indexBuffer, 0);
}

void Renderable::setElementCount(uint32_t count)
{
    elementCount = count;
}

}// namespace graphics
