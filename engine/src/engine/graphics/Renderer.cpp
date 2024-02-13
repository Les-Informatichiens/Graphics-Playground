//
// Created by Jonathan Richard on 2024-02-09.
//

#include "engine/graphics/Renderer.h"
#include "graphicsAPI/opengl/Device.h"

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
}

void Renderer::draw(Renderable& renderable)
{
    renderable.draw(*device, *activeCommandBuffer);
}

void Renderer::end()
{
    activeCommandBuffer->endRenderPass();
    activeCommandPool->submitCommandBuffer(activeCommandBuffer);
}

void Renderer::shutdown()
{
}
IDevice& Renderer::getDevice() const
{
    return *this->device;
}

}// namespace graphics
