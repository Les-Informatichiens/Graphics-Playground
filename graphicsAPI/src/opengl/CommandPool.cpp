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

std::unique_ptr<ICommandBuffer> opengl::CommandPool::acquireCommandBuffer(const CommandBufferDesc& desc)
{
    auto& pool = context->getCommandBufferPool();

    std::unique_ptr<ICommandBuffer> commandBuffer;
    if (pool.empty())
    {
        commandBuffer = std::make_unique<CommandBuffer>(context);
    }
    else
    {
        commandBuffer = std::move(pool[pool.size() - 1]);
        pool.pop_back();
    }
    ++activeCommandBufferCount;
    return commandBuffer;

//    auto commandBuffer = std::make_shared<CommandBuffer>(context);
//    ++activeCommandBufferCount;
//    return commandBuffer;
}

void opengl::CommandPool::submitCommandBuffer(std::unique_ptr<ICommandBuffer> commandBuffer)
{
    context->getCommandBufferPool().push_back(std::move(commandBuffer));
    --activeCommandBufferCount;
}
