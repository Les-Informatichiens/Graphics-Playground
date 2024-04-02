//
// Created by Jonathan Richard on 2024-02-07.
//

#include "Renderbuffer.h"
#include <stdexcept>

namespace opengl
{

Renderbuffer::Renderbuffer(Context& context, TextureFormat format)
    : Texture(context, format)
{
}

Renderbuffer::~Renderbuffer()
{
    if (handle != 0)
    {
        getContext().deleteRenderbuffers(1, &handle);
    }
}

void Renderbuffer::create(const TextureDesc& desc, bool hasStorageAlready)
{
    if (desc.usage != TextureDesc::TextureUsageBits::Attachment)
    {
        return;
    }

    FormatDescGL formatDescGl;
    if (!toFormatDescGL(getFormat(), desc.usage, formatDescGl))
    {
        return;
    }
    glInternalFormat = formatDescGl.format;

    getContext().genRenderbuffers(1, &handle);
    if (!hasStorageAlready)
    {
        getContext().bindRenderbuffer(GL_RENDERBUFFER, handle);
        if (desc.numSamples > 1)
        {
            getContext().renderbufferStorageMultisample(GL_RENDERBUFFER, desc.numSamples, glInternalFormat, Texture::getWidth(), Texture::getHeight());
        }
        else
        {
            getContext().renderbufferStorage(GL_RENDERBUFFER, glInternalFormat, Texture::getWidth(), Texture::getHeight());
        }
        getContext().bindRenderbuffer(GL_RENDERBUFFER, 0);
    }
}

void Renderbuffer::bind()
{
    getContext().bindRenderbuffer(GL_RENDERBUFFER, handle);
}

void Renderbuffer::unbind()
{
    getContext().bindRenderbuffer(GL_RENDERBUFFER, 0);
}

TextureType Renderbuffer::getType() const
{
    return TextureType::Texture2D;
}

size_t Renderbuffer::getUsage() const
{
    return TextureDesc::TextureUsageBits::Attachment;
}

Texture::BufferType Renderbuffer::getBufferType() const
{
    return Texture::BufferType::TextureBuffer;
}

GLuint Renderbuffer::getHandle() const
{
    return handle;
}

void Renderbuffer::bindImage(size_t unit, uint8_t accessFlags, uint32_t mipLevel, uint32_t layer)
{
    throw std::runtime_error("Renderbuffer does not support binding as image");
}

}// namespace opengl