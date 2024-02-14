//
// Created by Jonathan Richard on 2024-02-09.
//

#pragma once

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

    void begin();
    void draw(Renderable& renderable);
    void end();

    void shutdown();

    void setCamera(std::shared_ptr<Camera> camera);
    [[nodiscard]] Camera& getCamera() const;

    [[nodiscard]] IDevice& getDevice() const;
    [[nodiscard]] bool isInitialized() const { return initialized; }

private:
    std::unique_ptr<IDevice> device;
    std::shared_ptr<ICommandPool> activeCommandPool;
    std::unique_ptr<ICommandBuffer> activeCommandBuffer;

    std::shared_ptr<Camera> activeCamera;

    bool initialized = false;
};

}// namespace graphics