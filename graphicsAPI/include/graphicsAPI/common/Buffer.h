//
// Created by Jonathan Richard on 2024-01-31.
//

#pragma once

#include <cstdint>

struct BufferDesc
{
    using BufferType = uint8_t;

    enum BufferTypeBits : uint8_t {
        Index = 1 << 0,
        Vertex = 1 << 1,
        Uniform = 1 << 2,
        Storage = 1 << 3,
        Indirect = 1 << 4,
    };

    BufferType type;
};

class IBuffer
{
public:
    virtual ~IBuffer() = default;
    virtual void map(const void* data, unsigned int size) const = 0;
    virtual void mapRange(const void* data, unsigned int size, unsigned int offset) const = 0;
    virtual void unmap() const = 0;
};