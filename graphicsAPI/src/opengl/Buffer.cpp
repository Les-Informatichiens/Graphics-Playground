//
// Created by Jonathan Richard on 2024-01-31.
//

#include "graphicsAPI/opengl/Buffer.h"

namespace opengl {

Buffer::Buffer(Context& context, const BufferDesc& desc)
    : WithContext(context)
{
}

void Buffer::map(const void* data, unsigned int size) const
{
}

void Buffer::mapRange(const void* data, unsigned int size, unsigned int offset) const
{
}

void Buffer::unmap() const
{
}

}// namespace opengl
