//
// Created by Jonathan Richard on 2024-03-28.
//

#pragma once

#include "graphicsAPI/common/Buffer.h"
#include "graphicsAPI/common/Texture.h"
#include "graphicsAPI/common/SamplerState.h"
#include "graphicsAPI/common/ComputePipeline.h"

#include <memory>

struct ThreadGroupDimensions {
    uint32_t x;
    uint32_t y;
    uint32_t z;
};

class IComputeCommandBuffer
{
public:
    virtual ~IComputeCommandBuffer() = default;

    virtual void begin() = 0;
    virtual void end() = 0;

    virtual void bindComputePipeline(const std::shared_ptr<IComputePipeline>& pipeline) = 0;

    virtual void dispatch(const ThreadGroupDimensions& dimensions) = 0;

    virtual void bindBuffer(size_t index, std::shared_ptr<IBuffer> buffer, size_t offset) = 0;
    virtual void bindImage(size_t index, std::shared_ptr<ITexture> texture, uint8_t accessFlags, uint32_t mipLevel = 0, uint32_t layer = 0) = 0;
    virtual void bindTexture(size_t index, std::shared_ptr<ITexture> texture) = 0;
    virtual void bindSamplerState(size_t index, std::shared_ptr<ISamplerState> samplerState) = 0;
};
