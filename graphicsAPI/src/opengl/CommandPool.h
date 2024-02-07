//
// Created by Jonathan Richard on 2024-02-05.
//

#pragma once

#include "graphicsAPI/common/CommandPool.h"
#include "graphicsAPI/opengl/Context.h"

namespace opengl {

class CommandPool : public ICommandPool
{
public:
    explicit CommandPool(const std::shared_ptr<Context>& context, const CommandPoolDesc& desc);

    std::shared_ptr<ICommandBuffer> acquireCommandBuffer(const CommandBufferDesc& desc) override;
    void submitCommandBuffer(std::shared_ptr<ICommandBuffer> commandBuffer) override;

private:
    std::shared_ptr<Context> context;
    uint32_t activeCommandBufferCount = 0;
};

}// namespace opengl
