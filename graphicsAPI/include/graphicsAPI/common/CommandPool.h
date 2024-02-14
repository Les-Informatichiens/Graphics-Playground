//
// Created by Jonathan Richard on 2024-01-31.
//

#pragma once

#include "CommandBuffer.h"
#include <memory>

struct CommandPoolDesc
{
};

class ICommandPool
{
public:
    virtual ~ICommandPool() = default;
    virtual std::unique_ptr<ICommandBuffer> acquireCommandBuffer(const CommandBufferDesc& desc) = 0;
    virtual void submitCommandBuffer(std::unique_ptr<ICommandBuffer> commandBuffer) = 0;
};