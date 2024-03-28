//
// Created by Jonathan Richard on 2024-01-31.
//

#pragma once

#include "GraphicsCommandBuffer.h"
#include "ComputeCommandBuffer.h"
#include <memory>

struct CommandPoolDesc
{
};

class ICommandPool
{
public:
    virtual ~ICommandPool() = default;

    virtual std::unique_ptr<IGraphicsCommandBuffer> acquireGraphicsCommandBuffer(const CommandBufferDesc& desc) = 0;
    virtual void submitCommandBuffer(std::unique_ptr<IGraphicsCommandBuffer> commandBuffer) = 0;

    virtual std::unique_ptr<IComputeCommandBuffer> acquireComputeCommandBuffer(const CommandBufferDesc& desc) = 0;
    virtual void submitCommandBuffer(std::unique_ptr<IComputeCommandBuffer> commandBuffer) = 0;
};