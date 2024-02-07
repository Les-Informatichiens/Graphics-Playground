//
// Created by Jonathan Richard on 2024-02-07.
//

#pragma once

#include <cstdint>

#include "graphicsAPI/opengl/Context.h"
#include "graphicsAPI/opengl/Buffer.h"
#include <memory>
#include <unordered_map>

namespace opengl {

class UniformBinder
{
public:
    UniformBinder();
    ~UniformBinder() = default;

    void setBuffer(uint32_t index, const std::shared_ptr<Buffer>& buffer, uint32_t offset);
    void bindBuffers(Context& context);
    void clearDirtyBufferCache();

private:
    static const uint32_t MAX_UNIFORM_BUFFERS = 16;

    std::unordered_map<uint32_t, std::pair<std::shared_ptr<Buffer>, uint32_t>> uniformBufferCache;
    uint32_t uniformBuffersDirtyMask = 0;
};

}// namespace opengl
