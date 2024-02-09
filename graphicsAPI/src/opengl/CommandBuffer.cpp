//
// Created by Jonathan Richard on 2024-02-05.
//

#include "CommandBuffer.h"


#include "Framebuffer.h"
#include "graphicsAPI/opengl/Buffer.h"

namespace opengl {

static GLenum toOpenGLPrimitiveType(PrimitiveType type)
{
    switch (type)
    {
    case PrimitiveType::Point:
        return GL_POINTS;
    case PrimitiveType::Line:
        return GL_LINES;
    case PrimitiveType::LineStrip:
        return GL_LINE_STRIP;
    case PrimitiveType::Triangle:
        return GL_TRIANGLES;
    case PrimitiveType::TriangleStrip:
        return GL_TRIANGLE_STRIP;
    default:
        return GL_TRIANGLES;
    }
}

static GLenum toOpenGLIndexFormat(IndexFormat format)
{
    switch (format)
    {
    case IndexFormat::UInt16:
        return GL_UNSIGNED_SHORT;
    case IndexFormat::UInt32:
        return GL_UNSIGNED_INT;
    default:
        return GL_UNSIGNED_INT;
    }
}

CommandBuffer::CommandBuffer(const std::shared_ptr<Context>& context)
{
    activeVAO = std::make_shared<VertexArrayObject>(*context);
    activeVAO->create();

    uniformBinder = UniformBinder();

    this->context = context;
}

void CommandBuffer::beginRenderPass(const RenderPassBeginDesc& renderPass)
{
    // save the current state
    scissorEnabled = context->isEnabled(GL_SCISSOR_TEST);
    context->disable(GL_SCISSOR_TEST);

    activeVAO->bind();

    if (renderPass.framebuffer)
    {
        const auto& glFramebuffer = std::static_pointer_cast<Framebuffer>(renderPass.framebuffer);
        glFramebuffer->bindForRenderPass(renderPass.renderPass);
        bindViewport(glFramebuffer->getViewport());
    }
    else
    {
        context->bindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    isRecordingRenderCommands = true;
}

void CommandBuffer::endRenderPass()
{
    // restore the previous state
    if (scissorEnabled)
    {
        context->enable(GL_SCISSOR_TEST);
    }

    if (activeGraphicsPipeline) {
        activeGraphicsPipeline->unbindVertexAttributes();
    }
    activeGraphicsPipeline = nullptr;

    vertexBuffersDirtyCache.clear();
    uniformBinder.clearDirtyBufferCache();

    isRecordingRenderCommands = false;
}

void CommandBuffer::bindGraphicsPipeline(std::shared_ptr<IGraphicsPipeline> pipeline)
{
    auto glPipeline = std::static_pointer_cast<GraphicsPipeline>(pipeline);
    activeGraphicsPipeline = glPipeline;
    setDirty(DirtyFlag::DirtyBits_GraphicsPipeline);
}

void CommandBuffer::bindBuffer(uint32_t index, std::shared_ptr<IBuffer> buffer, uint32_t offset)
{
    auto glBuffer = std::static_pointer_cast<Buffer>(buffer);
    auto bufferType = glBuffer->getType();

    if (bufferType == Buffer::Type::Attribute)
    {
        vertexBuffersCache.insert_or_assign(index, std::move(std::static_pointer_cast<Buffer>(buffer)));
        vertexBuffersDirtyCache.insert(index);
    }
    else if (bufferType == Buffer::Type::Uniform)
    {
        uniformBinder.setBuffer(index, glBuffer, offset);
    }
}

void CommandBuffer::draw(PrimitiveType primitiveType, size_t vertexStart, size_t vertexCount)
{
    prepareForDraw();
    context->drawArrays(toOpenGLPrimitiveType(primitiveType), vertexStart, vertexCount);
}

void CommandBuffer::drawIndexed(PrimitiveType primitiveType, size_t indexCount, IndexFormat indexFormat, IBuffer& indexBuffer, size_t indexBufferOffset)
{
    prepareForDraw();
    auto glBuffer = dynamic_cast<ArrayBuffer*>(&indexBuffer);
    glBuffer->bind();
    void* offsetPtr = reinterpret_cast<void*>(indexBufferOffset);
    context->drawElements(toOpenGLPrimitiveType(primitiveType), indexCount, toOpenGLIndexFormat(indexFormat), offsetPtr);
}

void CommandBuffer::prepareForDraw()
{
    // bind vertex buffers and graphics pipeline
    if (activeGraphicsPipeline)
    {
        for (auto& [index, buffer] : vertexBuffersCache)
        {
            if (vertexBuffersDirtyCache.contains(index))
            {
                auto glBuffer = dynamic_cast<ArrayBuffer*>(buffer.get());
                glBuffer->bind();
                activeGraphicsPipeline->bindVertexAttributes(index, 0);
                vertexBuffersDirtyCache.erase(index);
            }
        }

        if (isDirty(DirtyFlag::DirtyBits_GraphicsPipeline))
        {
            activeGraphicsPipeline->bind();
            clearDirty(DirtyFlag::DirtyBits_GraphicsPipeline);
        }
    }

    if (activeDepthStencilState && isDirty(DirtyFlag::DirtyBits_DepthStencilState))
    {
        activeDepthStencilState->bind();
        clearDirty(DirtyFlag::DirtyBits_DepthStencilState);
    }

    // bind uniform buffers
    uniformBinder.bindBuffers(*context);
}

void CommandBuffer::clearPipelineResources(const std::shared_ptr<GraphicsPipeline>& newPipeline)
{
    auto currentGlPipeline = dynamic_cast<GraphicsPipeline*>(activeGraphicsPipeline.get());
    auto newGlPipeline = dynamic_cast<GraphicsPipeline*>(newPipeline.get());

    // later we can check if the pipeline is the same and skip the unbind/bind

    if (currentGlPipeline)
    {
        currentGlPipeline->unbind();
        currentGlPipeline->unbindVertexAttributes();
    }
}

void CommandBuffer::bindDepthStencilState(const std::shared_ptr<IDepthStencilState>& depthStencilState)
{
    activeDepthStencilState = std::static_pointer_cast<DepthStencilState>(depthStencilState);
    setDirty(DirtyFlag::DirtyBits_DepthStencilState);
}

void CommandBuffer::bindViewport(const Viewport& viewport)
{
    context->viewport(static_cast<GLint>(viewport.x), static_cast<GLint>(viewport.y), static_cast<GLint>(viewport.width), static_cast<GLint>(viewport.height));
}

void CommandBuffer::bindScissor(const Scissor& scissor)
{
    context->enable(GL_SCISSOR_TEST);
    context->scissor(static_cast<GLint>(scissor.x), static_cast<GLint>(scissor.y), static_cast<GLint>(scissor.width), static_cast<GLint>(scissor.height));
}

void CommandBuffer::bindTexture(uint32_t index, uint8_t target, std::shared_ptr<ITexture> texture)
{
}

bool CommandBuffer::isDirty(opengl::CommandBuffer::DirtyFlag flag) const
{
    return dirtyFlags & flag;
}
void CommandBuffer::setDirty(opengl::CommandBuffer::DirtyFlag flag)
{
    dirtyFlags |= flag;
}

void CommandBuffer::clearDirty(CommandBuffer::DirtyFlag flag)
{
    dirtyFlags &= ~flag;
}

}// namespace opengl