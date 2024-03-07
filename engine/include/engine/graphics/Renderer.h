//
// Created by Jonathan Richard on 2024-02-09.
//

#pragma once

#include "RenderTarget.h"
#include "Renderable.h"
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

    void shutdown();

    [[nodiscard]] IDevice& getDevice() const;
    [[nodiscard]] bool isInitialized() const { return initialized; }

private:
    std::unique_ptr<IDevice> device;
    std::shared_ptr<ICommandPool> activeCommandPool;
    std::unique_ptr<ICommandBuffer> activeCommandBuffer;
    std::shared_ptr<IFramebuffer> activeFramebuffer;

    bool initialized = false;
};

}// namespace graphics