//
// Created by Jonathan Richard on 2024-02-05.
//

#pragma once

#include "GraphicsPipeline.h"
#include "graphicsAPI/common/CommandBuffer.h"
#include "graphicsAPI/opengl/Context.h"

namespace opengl {

class CommandBuffer : public ICommandBuffer
{
public:
    explicit CommandBuffer(const std::shared_ptr<Context>& context);

    void beginRenderPass(const RenderPassDesc& renderPass) override;
    void endRenderPass() override;
    void bindGraphicsPipeline(std::shared_ptr<IGraphicsPipeline> pipeline) override;
    void bindBuffer(std::shared_ptr<IBuffer> buffer) override;
    void draw(PrimitiveType primitiveType, size_t vertexStart, size_t vertexCount) override;
    void drawIndexed(PrimitiveType primitiveType, size_t indexCount, IndexFormat indexFormat, IBuffer& indexBuffer, size_t indexBufferOffset) override;

private:
    void prepareForDraw();

private:
    std::shared_ptr<Context> context;

    // std::shared_ptr<Framebuffer> activeFramebuffer;
    std::shared_ptr<GraphicsPipeline> activeGraphicsPipeline;

    bool isRecordingRenderCommands = false;
    bool isRecordingComputeCommands = false;
};

}// namespace opengl