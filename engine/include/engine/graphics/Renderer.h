//
// Created by Jonathan Richard on 2024-02-09.
//

#pragma once

#include "Renderable.h"
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

    [[nodiscard]] IDevice& getDevice() const;
    [[nodiscard]] bool isInitialized() const { return initialized; }

private:
    std::unique_ptr<IDevice> device;
    std::shared_ptr<ICommandPool> activeCommandPool;
    std::shared_ptr<ICommandBuffer> activeCommandBuffer;

    bool initialized = false;
};

}// namespace graphics