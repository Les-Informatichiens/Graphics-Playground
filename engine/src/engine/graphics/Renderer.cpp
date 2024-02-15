//
// Created by Jonathan Richard on 2024-02-09.
//

#include "engine/graphics/Renderer.h"
#include "engine/MeshRenderer.h"
#include "graphicsAPI/opengl/Device.h"
#include <iostream>

namespace graphics {

void Renderer::initialize(graphics::RendererDesc desc)
{
    // for now we only support opengl
    auto oglContext = std::make_unique<opengl::Context>();

    this->device = std::make_unique<opengl::Device>(std::move(oglContext));

    this->activeCommandPool = device->createCommandPool({});

    this->initialized = true;
}

void Renderer::begin()
{
    activeCommandBuffer = activeCommandPool->acquireCommandBuffer({});

    RenderPassBeginDesc renderPassBegin = {
            .renderPass = {
                    .colorAttachments = {
                            {LoadAction::Clear, StoreAction::Store, {0.0f, 0.0f, 0.0f, 1.0f}}},
            },
            .framebuffer = nullptr};
    activeCommandBuffer->beginRenderPass(renderPassBegin);
//    std::cout << "begin" << std::endl;
}

void Renderer::draw(Renderable& renderable)
{
    renderable.draw(*device, *activeCommandBuffer);
}

void Renderer::end()
{
    activeCommandBuffer->endRenderPass();
    activeCommandPool->submitCommandBuffer(std::move(activeCommandBuffer));
//    std::cout << "end" << std::endl;
}

void Renderer::shutdown()
{
}

IDevice& Renderer::getDevice() const
{
    if (this->device == nullptr)
    {
        throw std::runtime_error("The device has not been initialized yet. Did you forget to call initialize?");
    }
    return *this->device;
}

void Renderer::setCamera(std::shared_ptr<Camera> camera)
{
    this->activeCamera = std::move(camera);
}

Camera& Renderer::getCamera() const
{
    if (this->activeCamera == nullptr)
    {
        throw std::runtime_error("No camera has been set");
    }
    return *this->activeCamera;
}

}// namespace graphics
