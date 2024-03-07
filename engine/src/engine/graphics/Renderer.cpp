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

void Renderer::begin(const graphics::RenderTarget& renderTarget)
{
    activeCommandBuffer = activeCommandPool->acquireCommandBuffer({});

    // create default depth texture
    auto depthAttachment = device->createTexture(TextureDesc::new2D(
            TextureFormat::Z_UNorm24,
            renderTarget.colorTexture->getWidth(),
            renderTarget.colorTexture->getHeight(),
            TextureDesc::TextureUsageBits::Attachment | TextureDesc::TextureUsageBits::Sampled));

    activeFramebuffer = device->createFramebuffer({
            .colorAttachments = {{0, {renderTarget.colorTexture, nullptr}}},
            .depthAttachment = {depthAttachment, nullptr}
    });

    RenderPassBeginDesc renderPassBegin = {
            .renderPass = {
                    .colorAttachments = {
                            {renderTarget.clear ? LoadAction::Clear : LoadAction::Load, StoreAction::Store, renderTarget.clearColor}},
                    .depthAttachment = RenderPassDesc::DepthAttachmentDesc{LoadAction::Clear, StoreAction::Store, 0, 0, 1.0f},

            },
            .framebuffer = activeFramebuffer
    };
    activeCommandBuffer->beginRenderPass(renderPassBegin);
}

void Renderer::draw(Renderable& renderable)
{
    renderable.draw(*device, *activeCommandBuffer);
}

void Renderer::end()
{
    activeCommandBuffer->endRenderPass();
    activeCommandPool->submitCommandBuffer(std::move(activeCommandBuffer));

    activeFramebuffer.reset();

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
void Renderer::bindViewport(const Viewport& viewport)
{
    this->activeCommandBuffer->bindViewport(viewport);
}

}// namespace graphics
