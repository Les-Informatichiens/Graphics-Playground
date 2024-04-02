//
// Created by Jonathan Richard on 2024-02-07.
//

#pragma once

#include "graphicsAPI/opengl/Context.h"
#include "graphicsAPI/common/Texture.h"
#include <limits>
#include <iostream>

namespace opengl
{

struct FormatDescGL final {
    GLint internalFormat = 0;
    GLenum format = GL_NONE;
    GLenum type = GL_NONE;
};

class Texture : public ITexture, public WithContext
{
public:
    enum class BufferType
    {
        TextureBuffer,
        Renderbuffer
    };

    explicit Texture(Context& context, TextureFormat format);

    void upload(const void* data, const TextureRangeDesc& range, size_t bytesPerRow) const override;
    void uploadCube(const void* data, TextureCubeFace face, const TextureRangeDesc& range, size_t bytesPerRow) const override;

    virtual void create(const TextureDesc& desc, bool hasStorageAlready) = 0;
    virtual void bind() =  0;
    virtual void bindImage(size_t unit, uint8_t accessFlags, uint32_t mipLevel, uint32_t layer) = 0;
    virtual void unbind() = 0;

    [[nodiscard]] virtual BufferType getBufferType() const = 0;
    [[nodiscard]] virtual GLuint getHandle() const = 0;

    static bool toFormatDescGL(TextureFormat textureFormat,
                               TextureDesc::TextureUsage usage,
                               FormatDescGL& outFormatGL);

    [[nodiscard]] float getAspectRatio() const override;
    [[nodiscard]] size_t getWidth() const override;
    [[nodiscard]] size_t getHeight() const override;
    [[nodiscard]] size_t getDepth() const override;
    [[nodiscard]] size_t getNumLayers() const override;
    [[nodiscard]] size_t getSamples() const override;
    [[nodiscard]] size_t getNumMipLevels() const override;
    [[nodiscard]] bool isRequiredGenerateMipmap() const override;
    void generateMipmap() const override;
    [[nodiscard]] TextureFormatProperties getProperties() const override;
    [[nodiscard]] TextureType getType() const override;
    [[nodiscard]] TextureFormat getFormat() const override;

    [[nodiscard]] TextureRangeDesc getFullRange(size_t mipLevel, size_t numMipLevels_) const override;
    [[nodiscard]] std::pair<bool, bool> validateRange(const TextureRangeDesc &range) const override;
    [[nodiscard]] GLint getAlignment(size_t stride, size_t mipLevel = 0) const;

    void setSamplerHash(size_t hash);
    [[nodiscard]] size_t getSamplerHash() const;

    static GLenum getTextureTarget(TextureType type, bool isMultisampled = false);

protected:
    size_t samplerHash = std::numeric_limits<size_t>::max();

    GLsizei width = 0;
    GLsizei height = 0;
    GLsizei depth = 1;
    GLsizei numLayers = 1;
    size_t numSamples = 1;
    size_t numMipLevels = 1;

    GLenum glInternalFormat;
    TextureType type = TextureType::Invalid;

    TextureFormatProperties formatProperties;
};

}// namespace opengl