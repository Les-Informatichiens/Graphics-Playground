//
// Created by Jonathan Richard on 2024-02-07.
//

#include "Framebuffer.h"
#include "Texture.h"
#include "TextureBuffer.h"

#include <algorithm>

namespace opengl
{

Framebuffer::Framebuffer(Context& context)
    : WithContext(context)
{
}

Framebuffer::~Framebuffer()
{
    if (framebuffer != 0)
    {
        getContext().deleteFramebuffers(1, &framebuffer);
        framebuffer = 0;
    }
}

void Framebuffer::updateDrawable(std::shared_ptr<ITexture> drawable)
{
    auto currentAttachment = getColorAttachment(0);

    if (drawable == nullptr && currentAttachment != nullptr)
    {
        auto& glCurrentAttachment = static_cast<Texture&>(*currentAttachment);
        getContext().bindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        detachColorTextureInternal(0, glCurrentAttachment.getBufferType());
        renderTarget.colorAttachments.erase(0);
    }

    if (drawable != nullptr)
    {
        getContext().bindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        attachColorTexture(0, drawable);
        renderTarget.colorAttachments[0].texture = drawable;
    }
}

void Framebuffer::create(const FramebufferDesc& desc)
{
    if (initialized)
    {
        return;
    }

    renderTarget = desc;
    initialized = true;

    // Create the framebuffer
    getContext().genFramebuffers(1, &framebuffer);
    getContext().bindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    std::vector<GLenum> drawBuffers;

    // Attach color textures
    for (const auto& [index, colorAttachment]: renderTarget.colorAttachments)
    {
        auto const& texture = colorAttachment.texture;
        attachColorTexture(static_cast<uint32_t>(index), texture);
        drawBuffers.push_back(static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + index));
    }
    std::sort(drawBuffers.begin(), drawBuffers.end());

    getContext().drawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());

    // Attach depth texture
    if (renderTarget.depthAttachment.texture)
    {
        auto const& texture = renderTarget.depthAttachment.texture;
        attachDepthTexture(texture);
    }

    // Attach stencil texture
    if (renderTarget.stencilAttachment.texture)
    {
        auto const& texture = renderTarget.stencilAttachment.texture;
        attachStencilTexture(texture);
    }

    if (GLenum fbStatus = getContext().checkFramebufferStatus(GL_FRAMEBUFFER); fbStatus != GL_FRAMEBUFFER_COMPLETE)
    {
        switch (fbStatus)
        {
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                throw std::runtime_error("Framebuffer incomplete: Attachment is NOT complete");
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                throw std::runtime_error("Framebuffer incomplete: No image is attached to FBO");
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                throw std::runtime_error("Framebuffer incomplete: Draw buffer");
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                throw std::runtime_error("Framebuffer incomplete: Read buffer");
            case GL_FRAMEBUFFER_UNSUPPORTED:
                throw std::runtime_error("Framebuffer incomplete: Unsupported by FBO implementation");
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                throw std::runtime_error("Framebuffer incomplete: Multisample");
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                throw std::runtime_error("Framebuffer incomplete: Layer targets");
            default:
                throw std::runtime_error("Framebuffer incomplete: Unknown error");
        }
    }

}

void Framebuffer::bindForRenderPass(const RenderPassDesc& renderPass) const
{
    activeRenderPass = renderPass;

    getContext().bindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    for (const auto& [index, colorAttachment]: renderTarget.colorAttachments)
    {
        auto const& texture = colorAttachment.texture;
        if (!texture)
        {
            continue;
        }

        if (texture->getProperties().isSRGB())
        {
            getContext().enable(GL_FRAMEBUFFER_SRGB);
        }
        else
        {
            getContext().disable(GL_FRAMEBUFFER_SRGB);
        }

        if (texture->getType() == TextureType::TextureCube)
        {
            attachColorTexture(index, texture, renderPass.colorAttachments[index].layer, renderPass.colorAttachments[index].mipmapLevel);
        }
    }

    GLbitfield clearMask;

    if (auto colorAttachment0 = renderTarget.colorAttachments.find(0);
        colorAttachment0 != renderTarget.colorAttachments.end() && colorAttachment0->second.texture != nullptr &&
        renderPass.colorAttachments[0].loadAction == LoadAction::Clear)
    {
        clearMask |= GL_COLOR_BUFFER_BIT;
        auto& clearColor = renderPass.colorAttachments[0].clearColor;
        getContext().colorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        getContext().clearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    }
    if (auto depthAttachment = renderTarget.depthAttachment.texture;
        depthAttachment != nullptr && renderPass.depthAttachment.loadAction == LoadAction::Clear)
    {
        clearMask |= GL_DEPTH_BUFFER_BIT;
        getContext().depthMask(GL_TRUE);
        getContext().clearDepth(renderPass.depthAttachment.clearDepth);
    }
    if (auto stencilAttachment = renderTarget.stencilAttachment.texture;
        stencilAttachment != nullptr && renderPass.stencilAttachment.loadAction == LoadAction::Clear)
    {
        clearMask |= GL_STENCIL_BUFFER_BIT;
        getContext().stencilMask(0xFF);
        getContext().clearStencil(renderPass.stencilAttachment.clearStencil);
    }

    if (clearMask != 0)
    {
        getContext().clear(clearMask);
    }
}

void Framebuffer::unbind() const
{
    GLenum attachments[3];
    GLsizei numAttachments = 0;

    auto colorAttachment0 = renderTarget.colorAttachments.find(0);

    if (colorAttachment0 != renderTarget.colorAttachments.end() &&
        colorAttachment0->second.texture != nullptr &&
        activeRenderPass.colorAttachments[0].storeAction != StoreAction::Store) {
        attachments[numAttachments++] = GL_COLOR_ATTACHMENT0;
    }
    if (renderTarget.depthAttachment.texture != nullptr) {
        if (activeRenderPass.depthAttachment.storeAction != StoreAction::Store) {
            attachments[numAttachments++] = GL_DEPTH_ATTACHMENT;
        }
    }
    if (renderTarget.stencilAttachment.texture != nullptr) {
        getContext().disable(GL_STENCIL_TEST);
        if (activeRenderPass.stencilAttachment.storeAction != StoreAction::Store) {
            attachments[numAttachments++] = GL_STENCIL_ATTACHMENT;
        }
    }

    if (numAttachments > 0) {
        getContext().invalidateFramebuffer(GL_FRAMEBUFFER, numAttachments, attachments);
    }
}

void Framebuffer::attachColorTexture(uint32_t index, const std::shared_ptr<ITexture>& texture, uint32_t face, uint32_t mipmapLevel) const
{
    auto& glTex = static_cast<Texture&>(*texture);
    Texture::BufferType bufferType = glTex.getBufferType();

    GLenum target;

    if (bufferType == Texture::BufferType::TextureBuffer)
    {
        auto& textureBuffer = dynamic_cast<TextureBuffer&>(*texture);
        GLenum glTexTarget = textureBuffer.getTarget();
        target = glTexTarget == GL_TEXTURE_CUBE_MAP ? GL_TEXTURE_CUBE_MAP_POSITIVE_X + face : glTexTarget;
    }
    attachColorTextureInternal(index, glTex.getHandle(), bufferType, target, glTex.getSamples(), face, mipmapLevel);
}

void Framebuffer::attachDepthTexture(const std::shared_ptr<ITexture>& texture) const
{
    auto& glTex = static_cast<Texture&>(*texture);
    attachDepthTextureInternal(glTex.getHandle(), glTex.getBufferType());
}

void Framebuffer::attachStencilTexture(const std::shared_ptr<ITexture>& texture) const
{
    auto& glTex = static_cast<Texture&>(*texture);
    attachStencilTextureInternal(glTex.getHandle(), glTex.getBufferType());
}

GLuint Framebuffer::getHandle() const
{
    return framebuffer;
}

bool Framebuffer::isInitialized() const
{
    return initialized;
}

std::vector<size_t> Framebuffer::getColorAttachmentIndices() const
{
    std::vector<size_t> indices;
    for (const auto& [index, _]: renderTarget.colorAttachments)
    {
        indices.push_back(index);
    }
    return indices;
}

std::shared_ptr<ITexture> Framebuffer::getColorAttachment(size_t index) const
{
    auto attachment = renderTarget.colorAttachments.find(index);
    if (attachment != renderTarget.colorAttachments.end())
    {
        return attachment->second.texture;
    }
    return nullptr;
}

std::shared_ptr<ITexture> Framebuffer::getDepthAttachment() const
{
    return renderTarget.depthAttachment.texture;
}

std::shared_ptr<ITexture> Framebuffer::getStencilAttachment() const
{
    return renderTarget.stencilAttachment.texture;
}

Viewport Framebuffer::getViewport() const
{
    // create a viewport from the base color attachment
    auto colorAttachment = getColorAttachment(0);
    if (colorAttachment)
    {
        return {0, 0, static_cast<float>(colorAttachment->getWidth()), static_cast<float>(colorAttachment->getHeight())};
    }
    return {};
}

void Framebuffer::attachColorTextureInternal(uint32_t index, GLuint textureID, Texture::BufferType bufferType, GLuint target, uint32_t numSamples, uint32_t face, uint32_t mipmapLevel) const
{
    if (bufferType == Texture::BufferType::Renderbuffer)
    {
        getContext().framebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_RENDERBUFFER, textureID);
    }
    else
    {
        GLenum finalTarget = target == GL_TEXTURE_CUBE_MAP ? GL_TEXTURE_CUBE_MAP_POSITIVE_X + face : target;
        if (numSamples > 1)
        {
            getContext().framebufferTexture2DMultisample(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, finalTarget, textureID, mipmapLevel, numSamples);
        }
        else
        {
            getContext().framebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, finalTarget, textureID, mipmapLevel);
        }
    }
}

void Framebuffer::attachDepthTextureInternal(GLuint textureID, Texture::BufferType bufferType) const
{
    if (bufferType == Texture::BufferType::Renderbuffer)
    {
        getContext().framebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, textureID);
    }
    else
    {
        getContext().framebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureID, 0);
    }
}
void Framebuffer::attachStencilTextureInternal(GLuint textureID, Texture::BufferType bufferType) const
{
    if (bufferType == Texture::BufferType::Renderbuffer)
    {
        getContext().framebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, textureID);
    }
    else
    {
        getContext().framebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, textureID, 0);
    }
}

void Framebuffer::detachColorTextureInternal(uint32_t index, Texture::BufferType bufferType, uint32_t mipmapLevel) const
{
    if (bufferType == Texture::BufferType::Renderbuffer)
    {
        getContext().framebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_RENDERBUFFER, 0);
    }
    else
    {
        getContext().framebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D, 0, mipmapLevel);
    }
}

void Framebuffer::detachDepthTextureInternal(Texture::BufferType bufferType) const
{
    if (bufferType == Texture::BufferType::Renderbuffer)
    {
        getContext().framebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
    }
    else
    {
        getContext().framebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
    }
}

void Framebuffer::detachStencilTextureInternal(Texture::BufferType bufferType) const
{
    if (bufferType == Texture::BufferType::Renderbuffer)
    {
        getContext().framebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
    }
    else
    {
        getContext().framebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
    }
}

}// namespace opengl