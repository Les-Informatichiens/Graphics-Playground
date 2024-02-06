//
// Created by Jonathan Richard on 2024-02-05.
//

#include "CommandBuffer.h"


#include "graphicsAPI/opengl/Buffer.h"

namespace opengl {

CommandBuffer::CommandBuffer(const std::shared_ptr<Context>& context)
{
    this->context = context;
}

void CommandBuffer::beginRenderPass(const RenderPassDesc& renderPass)
{

    isRecordingRenderCommands = true;
}

void CommandBuffer::endRenderPass()
{
    if (activeGraphicsPipeline) {
        activeGraphicsPipeline->unbindVertexAttributes();
    }
    activeGraphicsPipeline = nullptr;

    isRecordingRenderCommands = false;
}

void CommandBuffer::bindGraphicsPipeline(std::shared_ptr<IGraphicsPipeline> pipeline)
{
    auto glPipeline = std::static_pointer_cast<GraphicsPipeline>(pipeline);
    activeGraphicsPipeline = glPipeline;
}

void CommandBuffer::bindBuffer(std::shared_ptr<IBuffer> buffer)
{
    auto glBuffer = std::static_pointer_cast<Buffer>(buffer);
    auto bufferType = glBuffer->getType();

    if (bufferType == Buffer::Type::Attribute)
    {
        dynamic_cast<ArrayBuffer*>(buffer.get())->bind();
        // } else if (bufferType == Buffer::Type::Uniform) {
        // dynamic_cast<ArrayBuffer*>(buffer.get())->bind();
    }
}

void CommandBuffer::draw(PrimitiveType primitiveType, size_t vertexStart, size_t vertexCount)
{
}

void CommandBuffer::drawIndexed(PrimitiveType primitiveType, size_t indexCount, IndexFormat indexFormat, IBuffer& indexBuffer, size_t indexBufferOffset)
{
}

void CommandBuffer::prepareForDraw()
{
    if (activeGraphicsPipeline)
    {
        activeGraphicsPipeline->bindVertexAttributes(0, 0);
    }
}

}// namespace opengl