//
// Created by Jonathan Richard on 2024-01-31.
//

#pragma once

#include "Common.h"


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

    BufferType type = 0;
    const void* data = nullptr;
    uint32_t size = 0;
    ResourceStorage storage = ResourceStorage::Invalid;
};

class IBuffer
{
public:
    virtual ~IBuffer() = default;
    virtual void data(const void* data, uint32_t size, uint32_t offset) const = 0;
    [[nodiscard]] virtual void* map(uint32_t size, uint32_t offset) const = 0;
    virtual void unmap() const = 0;
};