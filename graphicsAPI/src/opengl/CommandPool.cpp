//
// Created by Jonathan Richard on 2024-02-05.
//

#include "CommandPool.h"

#include "CommandBuffer.h"

#include <utility>

opengl::CommandPool::CommandPool(const std::shared_ptr<Context>& context, const CommandPoolDesc& desc)
{
    this->context = context;
}

std::shared_ptr<ICommandBuffer> opengl::CommandPool::acquireCommandBuffer(const CommandBufferDesc& desc)
{
    auto commandBuffer = std::make_shared<CommandBuffer>(context);
    ++activeCommandBufferCount;
    return commandBuffer;
}

void opengl::CommandPool::submitCommandBuffer(std::shared_ptr<ICommandBuffer> commandBuffer)
{
    --activeCommandBufferCount;
}
