//
// Created by Jonathan Richard on 2024-02-09.
//

#pragma once

#include "RenderTarget.h"
#include "Renderable.h"
#include "VertexData.h"
#include "engine/Camera.h"
#include "engine/SceneNode.h"
#include "engine/graphics/DeviceManager.h"
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


    std::shared_ptr<IGraphicsPipeline> acquireGraphicsPipeline(const GraphicsPipelineDesc& desc);

    void shutdown();

    [[nodiscard]] IDevice& getDevice() const;
    [[nodiscard]] DeviceManager& getDeviceManager() { return deviceManager; }
    [[nodiscard]] bool isInitialized() const { return initialized; }

private:
    DeviceManager deviceManager;
    std::shared_ptr<ICommandPool> activeCommandPool;
    std::unique_ptr<IGraphicsCommandBuffer> activeCommandBuffer;
    std::shared_ptr<IFramebuffer> activeFramebuffer;

    std::unordered_map<size_t /*pipeline hash*/, std::shared_ptr<IGraphicsPipeline>> graphicsPipelines;

    bool initialized = false;
};

}// namespace graphics