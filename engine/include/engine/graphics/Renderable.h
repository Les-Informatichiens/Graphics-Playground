//
// Created by Jonathan Richard on 2024-02-12.
//

#pragma once

#include "graphicsAPI/common/Device.h"
#include "graphicsAPI/common/CommandBuffer.h"

namespace graphics {

class Renderable
{
public:
    Renderable(std::shared_ptr<IGraphicsPipeline> graphicsPipeline);

    void setElementCount(uint32_t count);

    void setVertexData(std::shared_ptr<IBuffer> buffer, std::shared_ptr<IBuffer> indexBuffer, uint32_t elementCount_);
    void setUniformBuffer(uint32_t binding, std::shared_ptr<IBuffer> buffer);
    void setDepthStencilState(std::shared_ptr<IDepthStencilState> depthStencilState);

    void draw(IDevice& device, ICommandBuffer& cmdBuffer) const;

private:
    std::shared_ptr<IBuffer> vertexBuffer;
    std::shared_ptr<IBuffer> indexBuffer;
    std::unordered_map<uint32_t, std::shared_ptr<IBuffer>> uniformBuffers;
    std::shared_ptr<IDepthStencilState> depthStencilState;

    std::shared_ptr<IGraphicsPipeline> graphicsPipeline;

    uint32_t elementCount;
};

} // namespace graphics