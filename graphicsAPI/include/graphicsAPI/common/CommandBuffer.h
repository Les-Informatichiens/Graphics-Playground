//
// Created by Jonathan Richard on 2024-01-31.
//

#pragma once

#include "GraphicsPipeline.h"
#include "RenderPass.h"
#include "Common.h"
#include "Buffer.h"

struct CommandBufferDesc
{
};

class ICommandBuffer
{
public:
    virtual ~ICommandBuffer() = default;

    virtual void beginRenderPass(const RenderPassDesc& renderPass) = 0;
    virtual void endRenderPass() = 0;
    virtual void bindGraphicsPipeline(std::shared_ptr<IGraphicsPipeline> pipeline) = 0;
    virtual void bindBuffer(uint32_t index, std::shared_ptr<IBuffer> buffer, uint32_t offset) = 0;
    virtual void draw(PrimitiveType primitiveType, size_t vertexStart, size_t vertexCount) = 0;
    virtual void drawIndexed(PrimitiveType primitiveType,
                     size_t indexCount,
                     IndexFormat indexFormat,
                     IBuffer& indexBuffer,
                     size_t indexBufferOffset) = 0;
};
