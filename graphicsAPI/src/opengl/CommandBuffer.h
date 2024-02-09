//
// Created by Jonathan Richard on 2024-02-05.
//

#pragma once

#include "DepthStencilState.h"
#include "GraphicsPipeline.h"
#include "UniformBinder.h"
#include "VertexArrayObject.h"
#include "graphicsAPI/common/CommandBuffer.h"
#include "graphicsAPI/opengl/Buffer.h"
#include "graphicsAPI/opengl/Context.h"

#include <set>

namespace opengl {

class CommandBuffer : public ICommandBuffer
{
public:
    enum DirtyFlag : uint32_t
    {
        DirtyBits_None = 0,
        DirtyBits_GraphicsPipeline = 1 << 1,
        DirtyBits_DepthStencilState = 1 << 2,
        DirtyBits_Viewport = 1 << 3,
        DirtyBits_Scissor = 1 << 4,
    };

    explicit CommandBuffer(const std::shared_ptr<Context>& context);

    void beginRenderPass(const RenderPassBeginDesc& renderPass) override;
    void endRenderPass() override;
    void bindGraphicsPipeline(std::shared_ptr<IGraphicsPipeline> pipeline) override;
    void bindBuffer(uint32_t index, std::shared_ptr<IBuffer> buffer, uint32_t offset) override;
    void draw(PrimitiveType primitiveType, size_t vertexStart, size_t vertexCount) override;
    void drawIndexed(PrimitiveType primitiveType, size_t indexCount, IndexFormat indexFormat, IBuffer& indexBuffer, size_t indexBufferOffset) override;
    void bindViewport(const Viewport& viewport) override;
    void bindScissor(const Scissor& scissor) override;
    void bindDepthStencilState(const std::shared_ptr<IDepthStencilState>& depthStencilState) override;
    void bindTexture(uint32_t index, uint8_t target, std::shared_ptr<ITexture> texture) override;

private:
    void clearPipelineResources(const std::shared_ptr<GraphicsPipeline>& newPipeline);
    void prepareForDraw();

    bool isDirty(DirtyFlag flag) const;
    void setDirty(DirtyFlag flag);
    void clearDirty(DirtyFlag flag);

private:
    std::shared_ptr<Context> context;
    std::set<uint32_t> vertexBuffersDirtyCache;
    std::unordered_map<uint32_t, std::shared_ptr<Buffer>> vertexBuffersCache;

    // std::shared_ptr<Framebuffer> activeFramebuffer;
    std::shared_ptr<GraphicsPipeline> activeGraphicsPipeline;
    std::shared_ptr<VertexArrayObject> activeVAO;
    std::shared_ptr<DepthStencilState> activeDepthStencilState;

    UniformBinder uniformBinder;

    uint32_t dirtyFlags = DirtyFlag::DirtyBits_None;

    bool scissorEnabled = false;

    bool isRecordingRenderCommands = false;
    bool isRecordingComputeCommands = false;
};

}// namespace opengl