//
// Created by Jonathan Richard on 2024-02-09.
//

#pragma once

#include "RenderTarget.h"
#include "Renderable.h"
#include "VertexData.h"
#include "engine/Camera.h"
#include "engine/SceneNode.h"
#include <graphicsAPI/common/Device.h>
#include <memory>

namespace graphics {

struct RendererDesc
{

};

class Renderer
{
public:
    Renderer() = default;
    ~Renderer() = default;

    void initialize(RendererDesc desc);

    void begin(const RenderTarget& renderTarget);
    void begin();
    void bindViewport(const Viewport& viewport);
    void draw(Renderable& renderable);
    void end();

    std::shared_ptr<Material> createMaterial(const std::shared_ptr<ShaderProgram>& shaderProgram);
    std::shared_ptr<ShaderProgram> createShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource);

    [[nodiscard]] std::shared_ptr<VertexData> createIndexedVertexData(const VertexDataLayout& layout, IndexFormat indexFormat, uint32_t vertexCount = 0, uint32_t indexCount = 0) const;
    std::shared_ptr<VertexData> createCachedIndexedVertexData(size_t id, const VertexDataLayout& layout, IndexFormat indexFormat, uint32_t vertexCount = 0, uint32_t indexCount = 0);

    [[nodiscard]] std::shared_ptr<VertexData> createVertexData(const VertexDataLayout& layout, uint32_t vertexCount = 0) const;
    std::shared_ptr<VertexData> createCachedVertexData(size_t id, const VertexDataLayout& layout, uint32_t vertexCount = 0);

    std::shared_ptr<IGraphicsPipeline> acquireGraphicsPipeline(const GraphicsPipelineDesc& desc);

    void shutdown();

    [[nodiscard]] IDevice& getDevice() const;
    [[nodiscard]] bool isInitialized() const { return initialized; }

private:
    std::unique_ptr<IDevice> device;
    std::shared_ptr<ICommandPool> activeCommandPool;
    std::unique_ptr<ICommandBuffer> activeCommandBuffer;
    std::shared_ptr<IFramebuffer> activeFramebuffer;

    std::unordered_map<size_t /*pipeline hash*/, std::shared_ptr<IGraphicsPipeline>> graphicsPipelines;
    std::unordered_map<size_t, std::shared_ptr<VertexData>> vertexDataCache;

    bool initialized = false;
};

}// namespace graphics