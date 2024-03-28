//
// Created by Jonathan Richard on 2024-01-31.
//

#pragma once

#include "Buffer.h"
#include "Common.h"
#include "DepthStencilState.h"
#include "Framebuffer.h"
#include "GraphicsPipeline.h"
#include "RenderPass.h"
#include "SamplerState.h"

struct CommandBufferDesc
{
};

struct RenderPassBeginDesc
{
    RenderPassDesc renderPass;
    std::shared_ptr<IFramebuffer> framebuffer;
};


enum BindTarget : uint8_t
{
    BindTarget_Vertex = 1 << 1,
    BindTarget_Fragment = 1 << 2,
};

class IGraphicsCommandBuffer
{
public:
    virtual ~IGraphicsCommandBuffer() = default;

    virtual void beginRenderPass(const RenderPassBeginDesc& renderPass) = 0;
    virtual void endRenderPass() = 0;
    virtual void bindGraphicsPipeline(std::shared_ptr<IGraphicsPipeline> pipeline) = 0;
    virtual void bindBuffer(uint32_t index, std::shared_ptr<IBuffer> buffer, uint32_t offset) = 0;
    virtual void draw(PrimitiveType primitiveType, size_t vertexStart, size_t vertexCount) = 0;
    virtual void drawIndexed(PrimitiveType primitiveType,
                     size_t indexCount,
                     IndexFormat indexFormat,
                     IBuffer& indexBuffer,
                     size_t indexBufferOffset) = 0;
    virtual void bindViewport(const Viewport& viewport) = 0;
    virtual void bindScissor(const ScissorRect& scissor) = 0;
    virtual void bindDepthStencilState(const std::shared_ptr<IDepthStencilState>& depthStencilState) = 0;
    virtual void bindTexture(uint32_t index, uint8_t target, std::shared_ptr<ITexture> texture) = 0;
    virtual void bindSamplerState(uint32_t index, uint8_t target, std::shared_ptr<ISamplerState> samplerState) = 0;

};
