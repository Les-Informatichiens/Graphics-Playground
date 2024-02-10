//
// Created by Jonathan Richard on 2024-02-09.
//

#pragma once

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
    void render();
    void shutdown();

    [[nodiscard]] IDevice& getDevice() const;
    [[nodiscard]] bool isInitialized() const { return initialized; }

private:
    std::unique_ptr<IDevice> device;

    bool initialized = false;
};

}// namespace graphics