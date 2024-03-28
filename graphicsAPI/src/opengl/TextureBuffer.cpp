//
// Created by Jonathan Richard on 2024-02-07.
//

#include "TextureBuffer.h"

namespace opengl {

const GLenum sCubeFaceTargets[6] = {GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                                    GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                                    GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
                                    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                                    GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
                                    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z};


TextureBuffer::TextureBuffer(Context& context, TextureFormat format)
    : Texture(context, format)
{
}

TextureBuffer::~TextureBuffer()
{
    if (handle != 0)
    {
        getContext().deleteTextures(1, &handle);
    }
}

void TextureBuffer::create(const TextureDesc& desc, bool hasStorageAlready)
{
    if (isInitialized)
        return;

    width = desc.width;
    height = desc.height;
    depth = desc.depth;
    type = desc.type;
    numLayers = desc.numLayers;
    numSamples = desc.numSamples;
    numMipLevels = desc.numMipLevels;

    auto isSampledOrStorage = (desc.usage & (TextureDesc::TextureUsageBits::Sampled | TextureDesc::TextureUsageBits::Storage)) != 0;
    if (isSampledOrStorage || desc.type != TextureType::Texture2D)
    {
        target = getTextureTarget(desc.type, numSamples > 1);
        if (target == 0)
        {
            return;
        }

        if (!toFormatDescGL(desc.format, desc.usage, formatDescGL))
        {
            return;
        }

        glInternalFormat = formatDescGL.internalFormat;

        getContext().genTextures(1, &handle);
        usage = desc.usage;

        getContext().bindTexture(target, handle);
        setMaxMipLevel();
        if (numMipLevels == 1)
        {
            getContext().texParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        }
        if (!useTexStorage())
        {
            initializeWithoutStorage();
        }
        else
        {
            initializeWithStorage();
        }
        getContext().bindTexture(target, 0);
    }

    isInitialized = true;
}

void TextureBuffer::upload(const void* data, const TextureRangeDesc& range, size_t bytesPerRow) const
{
    if (data == nullptr)
    {
        return;
    }
    getContext().bindTexture(target, handle);
    upload(target, range, data, bytesPerRow);
    getContext().bindTexture(target, 0);
}

void TextureBuffer::uploadCube(const void* data, TextureCubeFace face, const TextureRangeDesc& range, size_t bytesPerRow) const
{
    if (target != GL_TEXTURE_CUBE_MAP)
    {
        return;
    }
    if (data == nullptr)
    {
        return;
    }

    getContext().pixelStorei(GL_UNPACK_ALIGNMENT, getAlignment(bytesPerRow, range.mipLevel));
    getContext().bindTexture(target, handle);
    GLenum cubeTarget = sCubeFaceTargets[static_cast<size_t>(face)];
    upload2D(cubeTarget, range, data);
    getContext().bindTexture(target, 0);
}

Texture::BufferType TextureBuffer::getBufferType() const
{
    return Texture::BufferType::TextureBuffer;
}

GLuint TextureBuffer::getHandle() const
{
    return handle;
}

GLenum TextureBuffer::getTarget() const
{
    return target;
}

void TextureBuffer::bind()
{
    getContext().bindTexture(target, handle);
}

void TextureBuffer::bindImage(size_t unit)
{
    assert(getUsage() & TextureDesc::TextureUsageBits::Storage && "TextureBuffer::bindImage: Texture must have storage to be bound as image");
    getContext().bindImageTexture(static_cast<GLuint>(unit), getHandle(), 0, getTarget() == GL_TEXTURE_2D ? GL_TRUE : GL_FALSE, 0, GL_READ_WRITE, glInternalFormat);
}

void TextureBuffer::unbind()
{
    getContext().bindTexture(target, 0);
}

TextureFormatProperties TextureBuffer::getProperties() const
{
    return formatProperties;
}

void TextureBuffer::initializeWithStorage()
{
    const auto range = getFullRange(0, numMipLevels);
    switch (getType())
    {
        case TextureType::Texture2D: {
            getContext().texStorage2D(target, range.numMipLevels, glInternalFormat, range.width, range.height);
            break;
        }
        case TextureType::Texture2DArray: {
            getContext().texStorage3D(target, range.numMipLevels, glInternalFormat, range.width, range.height, range.numLayers);
            break;
        }
        case TextureType::Texture3D: {
            getContext().texStorage3D(target, range.numMipLevels, glInternalFormat, range.width, range.height, range.depth);
            break;
        }
        case TextureType::TextureCube: {
            getContext().texStorage2D(target, range.numMipLevels, glInternalFormat, range.width, range.height);
            break;
        }
    }
}

void TextureBuffer::initializeWithoutStorage()
{
    for (size_t i = 0; i < numMipLevels; ++i)
    {
        const auto range = getFullRange(i, 1);
        upload(target, range, nullptr);
    }
}

bool TextureBuffer::useTexStorage() const
{
    return (usage & TextureDesc::TextureUsageBits::Storage) != 0;
}

void TextureBuffer::setMaxMipLevel()
{
    getContext().texParameteri(target, GL_TEXTURE_MAX_LEVEL, (GLint) (numMipLevels - 1));
}

void TextureBuffer::upload(GLenum target, const TextureRangeDesc& range, const void* data, size_t bytesPerRow) const
{
    getContext().pixelStorei(GL_UNPACK_ALIGNMENT, getAlignment(bytesPerRow, range.mipLevel));

    switch (type)
    {
        case TextureType::Texture2D: {
            upload2D(target, range, data);
            break;
        }
        case TextureType::Texture2DArray: {
            upload2DArray(target, range, data);
            break;
        }
        case TextureType::Texture3D: {
            upload3D(target, range, data);
            break;
        }
        case TextureType::TextureCube: {
            for (auto cubeTarget : sCubeFaceTargets)
            {
                upload2D(cubeTarget, range, data);
            }
            break;
        }
    }
}

void TextureBuffer::upload2D(GLenum target, const TextureRangeDesc& range, const void* data) const
{
    const auto [result, fullRange] = validateRange(range);
    if (!result) {
        return;
    }
    // Use TexImage when range covers full texture AND texture was not initialized with TexStorage
    const auto texImage = fullRange && !useTexStorage();
    if (data == nullptr || !getProperties().isCompressed()) {
        if (texImage) {
            getContext().texImage2D(target,
                                    (GLsizei)range.mipLevel,
                                    formatDescGL.internalFormat,
                                    (GLsizei)range.width,
                                    (GLsizei)range.height,
                                    0, // border
                                    formatDescGL.format,
                                    formatDescGL.type,
                                    data);
        } else {
            getContext().texSubImage2D(target,
                                       (GLsizei)range.mipLevel,
                                       (GLsizei)range.x,
                                       (GLsizei)range.y,
                                       (GLsizei)range.width,
                                       (GLsizei)range.height,
                                       formatDescGL.format,
                                       formatDescGL.type,
                                       data);
        }
    } else {
        const auto numCompressedBytes = getProperties().getBytesPerRange(range);
        if (texImage) {
            getContext().compressedTexImage2D(target,
                                              (GLint)range.mipLevel,
                                              formatDescGL.internalFormat,
                                              (GLsizei)range.width,
                                              (GLsizei)range.height,
                                              0, // border
                                              (GLsizei)numCompressedBytes, // TODO: does not work
                                                                           // for compressed
                                                                           // mipmaps
                                              data);
        } else {
            getContext().compressedTexSubImage2D(getTarget(),
                                                 (GLint)range.mipLevel,
                                                 (GLint)range.x,
                                                 (GLint)range.y,
                                                 (GLsizei)range.width,
                                                 (GLsizei)range.height,
                                                 formatDescGL.internalFormat,
                                                 (GLsizei)numCompressedBytes, // TODO: does not work
                                                                              // for compressed
                                                                              // mipmaps
                                                 data);
        }
    }
}

void TextureBuffer::upload2DArray(GLenum target_, const TextureRangeDesc& range, const void* data) const
{
const auto [result, fullRange] = validateRange(range);
    if (!result) {
        return;
    }
    // Use TexImage when range covers full texture AND texture was not initialized with TexStorage
    const auto texImage = fullRange && !useTexStorage();
    if (data == nullptr || !getProperties().isCompressed()) {
        if (texImage) {
            getContext().texImage3D(target_,
                                    (GLint)range.mipLevel,
                                    formatDescGL.internalFormat,
                                    (GLsizei)range.width,
                                    (GLsizei)range.height,
                                    (GLsizei)range.numLayers,
                                    0, // border
                                    formatDescGL.format,
                                    formatDescGL.type,
                                    data);
        } else {
            getContext().texSubImage3D(target_,
                                       (GLint)range.mipLevel,
                                       (GLint)range.x,
                                       (GLint)range.y,
                                       (GLint)range.z,
                                       (GLsizei)range.width,
                                       (GLsizei)range.height,
                                       (GLsizei)range.numLayers,
                                       formatDescGL.format,
                                       formatDescGL.type,
                                       data);
        }
    } else {
        const auto numCompressedBytes = getProperties().getBytesPerRange(range);
        if (texImage) {
            getContext().compressedTexImage3D(target_,
                                              (GLint)range.mipLevel,
                                              formatDescGL.internalFormat,
                                              (GLsizei)range.width,
                                              (GLsizei)range.height,
                                              (GLsizei)range.numLayers,
                                              0, // border
                                              (GLsizei)numCompressedBytes, // TODO: does not work
                                                                           // for compressed
                                                                           // mipmaps
                                              data);
        } else {
            getContext().compressedTexSubImage3D(getTarget(),
                                                 (GLint)range.mipLevel,
                                                 (GLint)range.x,
                                                 (GLint)range.y,
                                                 (GLint)range.z,
                                                 (GLsizei)range.width,
                                                 (GLsizei)range.height,
                                                 (GLsizei)range.numLayers,
                                                 formatDescGL.internalFormat,
                                                 (GLsizei)numCompressedBytes, // TODO: does not work
                                                                              // for compressed
                                                                              // mipmaps
                                                 data);
        }
    }

}

void TextureBuffer::upload3D(GLenum target_, const TextureRangeDesc& range, const void* data) const
{
    const auto [result, fullRange] = validateRange(range);
    if (!result) {
        return;
    }
    // Use TexImage when range covers full texture AND texture was not initialized with TexStorage
    const auto texImage = fullRange && !useTexStorage();
    if (data == nullptr || !getProperties().isCompressed()) {
        if (texImage) {
            getContext().texImage3D(target_,
                                    (GLint)range.mipLevel,
                                    formatDescGL.internalFormat,
                                    (GLsizei)range.width,
                                    (GLsizei)range.height,
                                    (GLsizei)range.depth,
                                    0, // border
                                    formatDescGL.format,
                                    formatDescGL.type,
                                    data);
        } else {
            getContext().texSubImage3D(target_,
                                       (GLint)range.mipLevel,
                                       (GLint)range.x,
                                       (GLint)range.y,
                                       (GLint)range.z,
                                       (GLsizei)range.width,
                                       (GLsizei)range.height,
                                       (GLsizei)range.depth,
                                       formatDescGL.format,
                                       formatDescGL.type,
                                       data);
        }
    } else {
        const auto numCompressedBytes = getProperties().getBytesPerRange(range);
        if (texImage) {
            getContext().compressedTexImage3D(target_,
                                              (GLint)range.mipLevel,
                                              formatDescGL.internalFormat,
                                              (GLsizei)range.width,
                                              (GLsizei)range.height,
                                              (GLsizei)range.depth,
                                              0, // border
                                              (GLsizei)numCompressedBytes, // TODO: does not work
                                                                           // for compressed
                                                                           // mipmaps
                                              data);
        } else {
            getContext().compressedTexSubImage3D(getTarget(),
                                                 (GLint)range.mipLevel,
                                                 (GLint)range.x,
                                                 (GLint)range.y,
                                                 (GLint)range.z,
                                                 (GLsizei)range.width,
                                                 (GLsizei)range.height,
                                                 (GLsizei)range.depth,
                                                 formatDescGL.internalFormat,
                                                 (GLsizei)numCompressedBytes, // TODO: does not work
                                                                              // for compressed
                                                                              // mipmaps
                                                 data);
        }
    }
}

void TextureBuffer::generateMipmap() const
{
    getContext().bindTexture(target, handle);
    getContext().generateMipmap(target);
    getContext().bindTexture(target, 0);
}

bool TextureBuffer::isRequiredGenerateMipmap() const
{
    return numMipLevels > 1;
}

void TextureBuffer::upload1D(GLenum target, const TextureRangeDesc& range, const void* data) const
{
    // todo
}

void TextureBuffer::upload1DArray(GLenum target, const TextureRangeDesc& range, const void* data) const
{
    // todo
}

size_t TextureBuffer::getUsage() const
{
    return usage;
}

TextureType TextureBuffer::getType() const
{
    // TODO: Handle compressed texture type
    switch (target) {
        case GL_TEXTURE_CUBE_MAP:
            return TextureType::TextureCube;
        case GL_TEXTURE_2D:
        case GL_TEXTURE_2D_MULTISAMPLE:
            return TextureType::Texture2D;
        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
            return TextureType::Texture2DArray;
            break;
        case GL_TEXTURE_3D:
            return TextureType::Texture3D;
            break;
        case GL_TEXTURE_EXTERNAL_OES:
            return TextureType::Invalid;
            break;
    }
    return TextureType::Invalid;
}

}// namespace opengl
