//
// Created by Jonathan Richard on 2024-02-12.
//

#include "engine/graphics/Renderable.h"

namespace graphics {

Renderable::Renderable(std::shared_ptr<IGraphicsPipeline> graphicsPipeline)
    : graphicsPipeline(std::move(graphicsPipeline))
{
}

void Renderable::setUniformBuffer(uint32_t binding, std::shared_ptr<IBuffer> buffer)
{
    uniformBuffers[binding] = std::move(buffer);
}

void Renderable::setVertexData(std::shared_ptr<IBuffer> buffer, std::shared_ptr<IBuffer> indexBuffer_, uint32_t elementCount_)
{
    vertexBuffer = std::move(buffer);
    indexBuffer = std::move(indexBuffer_);
    elementCount = elementCount_;
}

void Renderable::draw(IDevice& device, ICommandBuffer& cmdBuffer) const
{
    cmdBuffer.bindGraphicsPipeline(graphicsPipeline);
    cmdBuffer.bindDepthStencilState(depthStencilState);
    cmdBuffer.bindBuffer(0, vertexBuffer, 0);
    for (const auto& [binding, buffer] : uniformBuffers)
    {
        cmdBuffer.bindBuffer(binding, buffer, 0);
    }
    cmdBuffer.drawIndexed(PrimitiveType::Triangle, elementCount, IndexFormat::UInt32, *indexBuffer, 0);
}

void Renderable::setElementCount(uint32_t count)
{
    elementCount = count;
}

void Renderable::setDepthStencilState(std::shared_ptr<IDepthStencilState> depthStencilState)
{
    this->depthStencilState = std::move(depthStencilState);
}

}// namespace graphics
