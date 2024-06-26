//
// Created by Jonathan Richard on 2024-02-05.
//

#include "GraphicsCommandBuffer.h"


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

GraphicsCommandBuffer::GraphicsCommandBuffer(const std::shared_ptr<Context>& context)
{
    activeVAO = std::make_shared<VertexArrayObject>(*context);
    activeVAO->create();

    uniformBinder = UniformBinder();

    this->context = context;
}

void GraphicsCommandBuffer::beginRenderPass(const RenderPassBeginDesc& desc)
{
    // save the current state
    scissorEnabled = false;//context->isEnabled(GL_SCISSOR_TEST);
    context->disable(GL_SCISSOR_TEST);

    if (activeVAO)
    {
        activeVAO->bind();
    }

    if (desc.framebuffer)
    {
        const auto& glFramebuffer = std::static_pointer_cast<Framebuffer>(desc.framebuffer);
        glFramebuffer->bindForRenderPass(desc.renderPass);
        bindViewport(glFramebuffer->getViewport());
    }
    else
    {
        // for the moment we will do this for the default framebuffer
        context->bindFramebuffer(GL_FRAMEBUFFER, 0);
        GLbitfield clearMask = 0;

        auto& renderPass = desc.renderPass;

        if (renderPass.colorAttachments[0].loadAction == LoadAction::Clear)
        {
            clearMask |= GL_COLOR_BUFFER_BIT;
            auto& clearColor = renderPass.colorAttachments[0].clearColor;
            context->colorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            context->clearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        }
        if (renderPass.depthAttachment.loadAction == LoadAction::Clear)
        {
            clearMask |= GL_DEPTH_BUFFER_BIT;
            context->depthMask(GL_TRUE);
            context->clearDepth(renderPass.depthAttachment.clearDepth);
        }
        if (renderPass.stencilAttachment.loadAction == LoadAction::Clear)
        {
            clearMask |= GL_STENCIL_BUFFER_BIT;
            context->stencilMask(0xFF);
            context->clearStencil(renderPass.stencilAttachment.clearStencil);
        }

        if (clearMask != 0)
        {
            context->clear(clearMask);
        }
    }

    for (size_t i = 0; i < MAX_TEXTURE_UNITS; ++i)
    {
        freeTextureUnits.push(i);
    }

    isRecordingRenderCommands = true;
}

void GraphicsCommandBuffer::endRenderPass()
{
    // restore the previous state
    if (scissorEnabled)
    {
        context->enable(GL_SCISSOR_TEST);
    }

    if (activeGraphicsPipeline)
    {
        activeGraphicsPipeline->unbindVertexAttributes();
    }
    activeGraphicsPipeline = nullptr;
    activeDepthStencilState = nullptr;

    vertexBuffersDirtyCache.clear();
    uniformBinder.clearDirtyBufferCache();

    vertTexturesCache = {};
    fragTexturesCache = {};
    freeTextureUnits = {};

    vertTexturesDirtyCache.reset();
    fragTexturesDirtyCache.reset();
    dirtyFlags = DirtyFlag::DirtyBits_None;

    isRecordingRenderCommands = false;
}

void GraphicsCommandBuffer::bindGraphicsPipeline(std::shared_ptr<IGraphicsPipeline> pipeline)
{
    auto glPipeline = std::static_pointer_cast<GraphicsPipeline>(pipeline);
    activeGraphicsPipeline = glPipeline;
    setDirty(DirtyFlag::DirtyBits_GraphicsPipeline);
}

void GraphicsCommandBuffer::bindBuffer(uint32_t index, std::shared_ptr<IBuffer> buffer, uint32_t offset)
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

void GraphicsCommandBuffer::draw(PrimitiveType primitiveType, size_t vertexStart, size_t vertexCount)
{
    prepareForDraw();
    context->drawArrays(toOpenGLPrimitiveType(primitiveType), vertexStart, vertexCount);
}

void GraphicsCommandBuffer::drawIndexed(PrimitiveType primitiveType, size_t indexCount, IndexFormat indexFormat, IBuffer& indexBuffer, size_t indexBufferOffset)
{
    prepareForDraw();
    auto glBuffer = dynamic_cast<ArrayBuffer*>(&indexBuffer);
    glBuffer->bind();
    auto* offsetPtr = reinterpret_cast<GLvoid*>(indexBufferOffset);
    context->drawElements(toOpenGLPrimitiveType(primitiveType), indexCount, toOpenGLIndexFormat(indexFormat), offsetPtr);
}

void GraphicsCommandBuffer::prepareForDraw()
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

    if (activeGraphicsPipeline)
    {
        // bind uniform buffers
        uniformBinder.bindBuffers(*context);

        // TODO: bind textures and samplers
        for (size_t i = 0; i < MAX_TEXTURE_SAMPLERS; ++i)
        {
            if (!vertTexturesDirtyCache[i])
            {
                continue;
            }
            auto& textureState = vertTexturesCache[i];
            if (auto& texture = textureState.texture)
            {
//                activeGraphicsPipeline->bindTextureUnit(i, BindTarget::BindTarget_Vertex);
                int loc = activeGraphicsPipeline->getTextureUnitLocation(i, BindTarget::BindTarget_Vertex);
                if (loc >= 0)
                {
                    context->uniform1i(loc, static_cast<GLint>(i));
                    context->activeTexture(GL_TEXTURE0 + i);

//                context->uniform1i(static_cast<GLint>(i), static_cast<GLint>(textureState.textureUnit));
//                context->activeTexture(GL_TEXTURE0 + textureState.textureUnit);
                    texture->bind();
                    freeTextureUnits.push(textureState.textureUnit);
                    textureState.textureUnit = -1;

                    if (auto& sampler = textureState.samplerState)
                    {
                        sampler->bind(texture);
                    }
                    vertTexturesDirtyCache.reset(i);
                }
            }
        }
        for (size_t i = 0; i < MAX_TEXTURE_SAMPLERS; ++i)
        {
            if (!fragTexturesDirtyCache[i])
            {
                continue;
            }
            auto& textureState = fragTexturesCache[i];
            if (auto& texture = textureState.texture)
            {
//                activeGraphicsPipeline->bindTextureUnit(i, BindTarget::BindTarget_Fragment);
                int loc = activeGraphicsPipeline->getTextureUnitLocation(i, BindTarget::BindTarget_Fragment);
                if (loc >= 0)
                {
                    context->uniform1i(loc, static_cast<GLint>(i));
                    context->activeTexture(GL_TEXTURE0 + i);

//                context->uniform1i(static_cast<GLint>(i), static_cast<GLint>(textureState.textureUnit));
//                context->activeTexture(GL_TEXTURE0 + textureState.textureUnit);
                    texture->bind();
                    freeTextureUnits.push(textureState.textureUnit);
                    textureState.textureUnit = -1;

                    if (auto& sampler = textureState.samplerState)
                    {
                        sampler->bind(texture);
                    }
                    fragTexturesDirtyCache.reset(i);
                }
            }
        }
    }
}

void GraphicsCommandBuffer::clearPipelineResources(const std::shared_ptr<GraphicsPipeline>& newPipeline)
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

void GraphicsCommandBuffer::bindDepthStencilState(const std::shared_ptr<IDepthStencilState>& depthStencilState)
{
    activeDepthStencilState = std::static_pointer_cast<DepthStencilState>(depthStencilState);
    setDirty(DirtyFlag::DirtyBits_DepthStencilState);
}

void GraphicsCommandBuffer::bindViewport(const Viewport& viewport)
{
    context->viewport(static_cast<GLint>(viewport.x), static_cast<GLint>(viewport.y), static_cast<GLint>(viewport.width), static_cast<GLint>(viewport.height));
}

void GraphicsCommandBuffer::bindScissor(const ScissorRect& scissor)
{
    if (scissor.isNull())
    {
        context->disable(GL_SCISSOR_TEST);
        return;
    }
    context->enable(GL_SCISSOR_TEST);
    context->scissor(static_cast<GLint>(scissor.x), static_cast<GLint>(scissor.y), static_cast<GLint>(scissor.width), static_cast<GLint>(scissor.height));
}

void GraphicsCommandBuffer::bindTexture(uint32_t index, uint8_t target, std::shared_ptr<ITexture> texture)
{
    if (freeTextureUnits.empty())
    {
        throw std::runtime_error("No free texture units available");
    }
//    int unit = freeTextureUnits.front();
//    std::cout << "Bound texture to unit: " << unit << std::endl;
    if ((target & BindTarget::BindTarget_Vertex) != 0)
    {
        auto& texState = vertTexturesCache[index];
        texState.texture = std::static_pointer_cast<Texture>(texture);
        texState.textureUnit = index;//unit;
//        freeTextureUnits.pop();
        vertTexturesDirtyCache.set(index);
    }
    if ((target & BindTarget::BindTarget_Fragment) != 0)
    {
        auto& texState = fragTexturesCache[index];
        texState.texture = std::static_pointer_cast<Texture>(texture);
        texState.textureUnit = index;//unit;
//        freeTextureUnits.pop();
        fragTexturesDirtyCache.set(index);
    }
}

void GraphicsCommandBuffer::bindSamplerState(uint32_t index, uint8_t target, std::shared_ptr<ISamplerState> samplerState)
{
    if ((target & BindTarget::BindTarget_Vertex) != 0)
    {
        vertTexturesCache[index].samplerState = std::static_pointer_cast<SamplerState>(samplerState);
        vertTexturesDirtyCache.set(index);
    }
    if ((target & BindTarget::BindTarget_Fragment) != 0)
    {
        fragTexturesCache[index].samplerState = std::static_pointer_cast<SamplerState>(samplerState);
        fragTexturesDirtyCache.set(index);
    }
}

bool GraphicsCommandBuffer::isDirty(opengl::GraphicsCommandBuffer::DirtyFlag flag) const
{
    return dirtyFlags & flag;
}
void GraphicsCommandBuffer::setDirty(opengl::GraphicsCommandBuffer::DirtyFlag flag)
{
    dirtyFlags |= flag;
}
void GraphicsCommandBuffer::clearDirty(GraphicsCommandBuffer::DirtyFlag flag)
{
    dirtyFlags &= ~flag;
}

}// namespace opengl