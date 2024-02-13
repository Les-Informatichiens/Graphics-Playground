//
// Created by Jonathan Richard on 2024-02-07.
//

#include "UniformBinder.h"

namespace opengl {

UniformBinder::UniformBinder()
{
    uniformBufferCache.reserve(MAX_UNIFORM_BUFFERS);
    uniformBuffersDirtyMask = 0;
}

void UniformBinder::setBuffer(uint32_t index, const std::shared_ptr<Buffer>& buffer, uint32_t offset)
{
    if (index >= 0 && index < MAX_UNIFORM_BUFFERS)
    {
        uniformBufferCache[index] = std::make_pair(buffer, offset);
        uniformBuffersDirtyMask |= 1 << index;
    }
    else
    {
        // handle error
    }
}

void UniformBinder::bindBuffers(Context& context)
{
    for (auto& [bufferIndex, bindingInfo] : uniformBufferCache)
    {
        if (uniformBuffersDirtyMask & (1 << bufferIndex))
        {
            if (bindingInfo.second > 0)
            {
                auto* bufferObject = static_cast<ArrayBuffer*>(bindingInfo.first.get());
                bufferObject->bindRange(bufferIndex, bindingInfo.second);
            }
            else
            {
                auto* bufferObject = static_cast<ArrayBuffer*>(bindingInfo.first.get());
                bufferObject->bindBase(bufferIndex);
            }
        }
    }
    uniformBuffersDirtyMask = 0;
}

void UniformBinder::clearDirtyBufferCache()
{
    uniformBuffersDirtyMask = 0;
}

}// namespace opengl
