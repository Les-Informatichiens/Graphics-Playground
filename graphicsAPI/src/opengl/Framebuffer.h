//
// Created by Jonathan Richard on 2024-02-07.
//

#pragma once

#include "graphicsAPI/opengl/Context.h"
#include "graphicsAPI/common/Framebuffer.h"
#include "Texture.h"

namespace opengl
{

class Framebuffer : public IFramebuffer, public WithContext
{
public:
    explicit Framebuffer(Context& context);
    ~Framebuffer() override;

    void updateDrawable(std::shared_ptr<ITexture> drawable) override;

    void create(const FramebufferDesc& desc);

    void attachColorTexture(uint32_t index, const std::shared_ptr<ITexture>& texture, uint32_t face = 0, uint32_t mipmapLevel = 0) const;
    void attachDepthTexture(const std::shared_ptr<ITexture>& texture) const;
    void attachStencilTexture(const std::shared_ptr<ITexture>& texture) const;

    [[nodiscard]] std::vector<size_t> getColorAttachmentIndices() const override;
    [[nodiscard]] std::shared_ptr<ITexture> getColorAttachment(size_t index) const override;
    [[nodiscard]] std::shared_ptr<ITexture> getDepthAttachment() const override;
    [[nodiscard]] std::shared_ptr<ITexture> getStencilAttachment() const override;


    void bindForRenderPass(const RenderPassDesc& renderPass) const;
    void unbind() const;

    [[nodiscard]] Viewport getViewport() const;

    [[nodiscard]] GLuint getHandle() const;

    [[nodiscard]] bool isInitialized() const;

private:
    void attachColorTextureInternal(uint32_t index, GLuint textureID, Texture::BufferType bufferType, GLuint target, uint32_t numSamples = 1, uint32_t face = 0, uint32_t mipmapLevel = 0) const;
    void attachDepthTextureInternal(GLuint textureID, Texture::BufferType bufferType) const;
    void attachStencilTextureInternal(GLuint textureID, Texture::BufferType bufferType) const;
    void detachColorTextureInternal(uint32_t index, Texture::BufferType bufferType, uint32_t mipmapLevel = 0) const;
    void detachDepthTextureInternal(Texture::BufferType bufferType) const;
    void detachStencilTextureInternal(Texture::BufferType bufferType) const;

private:

    GLuint framebuffer;
    bool initialized = false;

    FramebufferDesc renderTarget; // attachments
    mutable RenderPassDesc activeRenderPass;
};

}
