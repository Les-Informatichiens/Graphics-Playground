//
// Created by Jonathan Richard on 2024-02-05.
//

#pragma once

#include "graphicsAPI/common/CommandPool.h"
#include "graphicsAPI/opengl/Context.h"

#include "graphicsAPI/common/Common.h"


namespace opengl {

class CommandPool : public ICommandPool
{
public:
    explicit CommandPool(const std::shared_ptr<Context>& context, const CommandPoolDesc& desc);

    std::unique_ptr<IGraphicsCommandBuffer> acquireGraphicsCommandBuffer(const CommandBufferDesc& desc) override;
    void submitCommandBuffer(std::unique_ptr<IGraphicsCommandBuffer> commandBuffer) override;

    std::unique_ptr<IComputeCommandBuffer> acquireComputeCommandBuffer(const CommandBufferDesc& desc) override;
    void submitCommandBuffer(std::unique_ptr<IComputeCommandBuffer> commandBuffer) override;

private:
    std::shared_ptr<Context> context;

    uint32_t activeCommandBufferCount = 0;
};

}// namespace opengl
