//
// Created by Jonathan Richard on 2024-02-05.
//

#pragma once

#include "GraphicsPipeline.h"
#include "VertexArrayObject.h"
#include "graphicsAPI/common/CommandBuffer.h"
#include "graphicsAPI/opengl/Buffer.h"
#include "graphicsAPI/opengl/Context.h"

#include <set>

namespace opengl {

class CommandBuffer : public ICommandBuffer
{
public:
    explicit CommandBuffer(const std::shared_ptr<Context>& context);

    void beginRenderPass(const RenderPassDesc& renderPass) override;
    void endRenderPass() override;
    void bindGraphicsPipeline(std::shared_ptr<IGraphicsPipeline> pipeline) override;
    void bindBuffer(uint32_t index, std::shared_ptr<IBuffer> buffer, uint32_t offset) override;
    void draw(PrimitiveType primitiveType, size_t vertexStart, size_t vertexCount) override;
    void drawIndexed(PrimitiveType primitiveType, size_t indexCount, IndexFormat indexFormat, IBuffer& indexBuffer, size_t indexBufferOffset) override;

private:
    void clearPipelineResources(const std::shared_ptr<GraphicsPipeline>& newPipeline);
    void prepareForDraw();

private:
    std::shared_ptr<Context> context;
    std::set<uint32_t> vertexBuffersDirtyCache;
    std::unordered_map<uint32_t, std::shared_ptr<Buffer>> vertexBuffersCache;

    // std::shared_ptr<Framebuffer> activeFramebuffer;
    std::shared_ptr<GraphicsPipeline> activeGraphicsPipeline;
    std::shared_ptr<VertexArrayObject> activeVAO;

    bool isRecordingRenderCommands = false;
    bool isRecordingComputeCommands = false;
};

}// namespace opengl