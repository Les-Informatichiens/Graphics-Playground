//
// Created by Jonathan Richard on 2024-02-07.
//

#pragma once

#include "Texture.h"

namespace opengl
{

class TextureBuffer : public Texture
{
public:
    explicit TextureBuffer(Context& context, TextureFormat format);
    ~TextureBuffer() override;

    void create(const TextureDesc& desc, bool hasStorageAlready) override;

    void upload(const void* data, const TextureRangeDesc& range, size_t bytesPerRow) const override;
    void uploadCube(const void* data, TextureCubeFace face, const TextureRangeDesc& range, size_t bytesPerRow) const override;

    [[nodiscard]] BufferType getBufferType() const override;
    [[nodiscard]] GLuint getHandle() const override;
    [[nodiscard]] TextureFormatProperties getProperties() const override;

    size_t getUsage() const override;
    [[nodiscard]] GLenum getTarget() const;
    [[nodiscard]] TextureType getType() const override;

    void bind() override;
    void bindImage(size_t unit, uint8_t accessFlags, uint32_t mipLevel, uint32_t layer) override;
    void unbind() override;

private:
    void initializeWithStorage();
    void initializeWithoutStorage();

    void upload(GLenum target,
                const TextureRangeDesc& range,
                const void* data,
                size_t bytesPerRow = 0) const;
    void upload1D(GLenum target, const TextureRangeDesc& range, const void* data) const;
    void upload1DArray(GLenum target, const TextureRangeDesc& range, const void* data) const;
    void upload2D(GLenum target, const TextureRangeDesc& range, const void* data) const;
    void upload2DArray(GLenum target_, const TextureRangeDesc& range, const void* data) const;
    void upload3D(GLenum target_, const TextureRangeDesc& range, const void* data) const;

    void setMaxMipLevel();
    [[nodiscard]] bool useTexStorage() const;

    bool isRequiredGenerateMipmap() const override;
    void generateMipmap() const override;

private:


    GLuint handle = 0;
    GLenum target;
    TextureDesc::TextureUsage usage;

    bool isInitialized = false;

    FormatDescGL formatDescGL;
};


}// namespace opengl