//
// Created by Jonathan Richard on 2024-02-05.
//

#include "CommandPool.h"

#include "ComputeCommandBuffer.h"
#include "GraphicsCommandBuffer.h"

#include <utility>

opengl::CommandPool::CommandPool(const std::shared_ptr<Context>& context, const CommandPoolDesc& desc)
{
    this->context = context;
}

std::unique_ptr<IGraphicsCommandBuffer> opengl::CommandPool::acquireGraphicsCommandBuffer(const CommandBufferDesc& desc)
{
    auto& pool = context->getGraphicsCommandBufferPool();

    std::unique_ptr<IGraphicsCommandBuffer> commandBuffer;
    if (pool.empty())
    {
        commandBuffer = std::make_unique<GraphicsCommandBuffer>(context);
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

void opengl::CommandPool::submitCommandBuffer(std::unique_ptr<IGraphicsCommandBuffer> commandBuffer)
{
    context->getGraphicsCommandBufferPool().push_back(std::move(commandBuffer));
    --activeCommandBufferCount;
}

std::unique_ptr<IComputeCommandBuffer> opengl::CommandPool::acquireComputeCommandBuffer(const CommandBufferDesc& desc)
{
    auto& pool = context->getComputeCommandBufferPool();

    std::unique_ptr<IComputeCommandBuffer> commandBuffer;
    if (pool.empty())
    {
        commandBuffer = std::make_unique<ComputeCommandBuffer>(context);
    }
    else
    {
        commandBuffer = std::move(pool[pool.size() - 1]);
        pool.pop_back();
    }
    ++activeCommandBufferCount;
    return commandBuffer;
}

void opengl::CommandPool::submitCommandBuffer(std::unique_ptr<IComputeCommandBuffer> commandBuffer)
{
    context->getComputeCommandBufferPool().push_back(std::move(commandBuffer));
    --activeCommandBufferCount;
}
