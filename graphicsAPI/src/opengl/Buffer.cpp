//
// Created by Jonathan Richard on 2024-01-31.
//

#include "graphicsAPI/opengl/Buffer.h"

namespace opengl {

ArrayBuffer::ArrayBuffer(Context& context)
    : Buffer(context), id_(0), isDynamic_(false), size_(0)
{
}

ArrayBuffer::~ArrayBuffer()
{
    if (id_ != 0)
    {
        getContext().deleteBuffers(1, &id_);
        getContext().unbindBuffer(getTarget());
        id_ = 0;
    }
    size_ = 0;
    isDynamic_ = false;
}

void ArrayBuffer::initialize(const BufferDesc& desc)
{
    GLenum usage = GL_DYNAMIC_DRAW;
    switch (desc.storage)
    {
        case ResourceStorage::Shared:
            usage = GL_DYNAMIC_DRAW;
            isDynamic_ = true;
            break;
        case ResourceStorage::Managed:
            usage = GL_STATIC_DRAW;
            isDynamic_ = false;
            break;
        case ResourceStorage::Private:
            usage = GL_STATIC_DRAW;
            isDynamic_ = false;
            break;
        default:
            break;
    }

    if (!isDynamic_ && desc.data == nullptr)
    {
        // cannot create a static buffer without data
        return;
    }

    getContext().genBuffers(1, &id_);

    if (desc.type & BufferDesc::BufferTypeBits::Storage)
    {
        // if (getContext().deviceFeatures().hasFeature(DeviceFeatures::Compute)) {
        target_ = GL_SHADER_STORAGE_BUFFER;
        type_ = Type::Storage;
    }
    else if (desc.type & BufferDesc::BufferTypeBits::Uniform)
    {
        target_ = GL_UNIFORM_BUFFER;
        type_ = Type::Uniform;
    }
    else if (desc.type & BufferDesc::BufferTypeBits::Vertex)
    {
        target_ = GL_ARRAY_BUFFER;
        type_ = Type::Attribute;
    }
    else if (desc.type & BufferDesc::BufferTypeBits::Index)
    {
        target_ = GL_ELEMENT_ARRAY_BUFFER;
        type_ = Type::Index;
    }
    else
    {
        // err, unknown buffer type
        return;
    }

    size_ = desc.size;
    savePreviousBuffer();
    getContext().bindBuffer(getTarget(), id_);
    getContext().bufferData(getTarget(), size_, desc.data, usage);
    getContext().bindBuffer(getTarget(), 0);
    restorePreviousBuffer();
}

void ArrayBuffer::data(const void* data, uint32_t size, uint32_t offset) const
{
    if (!isDynamic_)
    {
        // err, static buffers should not be written to
        return;
    }

    savePreviousBuffer();
    getContext().bindBuffer(target_, id_);
    if (offset == 0 && size == 0)
    {
        getContext().bufferData(target_, size_, data, GL_DYNAMIC_DRAW);
    }
    else
    {
        getContext().bufferSubData(target_, offset, size, data);
    }
    restorePreviousBuffer();
}

void* ArrayBuffer::map(uint32_t size, uint32_t offset) const
{
    getContext().bindBuffer(getTarget(), id_);
    return getContext().mapBufferRange(getTarget(), offset, size, GL_MAP_WRITE_BIT);
}

void ArrayBuffer::unmap() const
{
    getContext().bindBuffer(getTarget(), id_);
    getContext().unmapBuffer(getTarget());
    getContext().bindBuffer(getTarget(), 0);
}


void ArrayBuffer::bind() const noexcept
{
    getContext().bindBuffer(getTarget(), id_);
}

void ArrayBuffer::bindBase(uint32_t index) const noexcept
{
    if (target_ != GL_SHADER_STORAGE_BUFFER && target_ != GL_UNIFORM_BUFFER)
    {
        // err, can only bind to shader storage buffers or uniform buffers
        return;
    }
    savePreviousBuffer();
    getContext().bindBuffer(target_, id_);
    getContext().bindBufferBase(target_, (GLuint) index, id_);
    restorePreviousBuffer();
}

void ArrayBuffer::unbind() const noexcept
{
    getContext().bindBuffer(getTarget(), 0);
}

void ArrayBuffer::bindRange(uint32_t index, uint32_t offset) const noexcept
{
    if (target_ != GL_SHADER_STORAGE_BUFFER && target_ != GL_UNIFORM_BUFFER)
    {
        // err, can only bind to shader storage buffers or uniform buffers
        return;
    }
    savePreviousBuffer();
    getContext().bindBuffer(target_, id_);
    getContext().bindBufferRange(target_, (GLuint) index, id_, offset, size_ - offset);
    restorePreviousBuffer();
}

size_t ArrayBuffer::getSize() const
{
    return size_;
}

void ArrayBuffer::savePreviousBuffer() const
{
    // this method is a wrap around getContext().bindBuffer(getTarget(), id_); to ensure we dont alter the current state
    getContext().getIntegerv(target_, &lastBoundBuffer_);
}

void ArrayBuffer::restorePreviousBuffer() const
{
    getContext().bindBuffer(target_, lastBoundBuffer_);
}

}// namespace opengl
