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

    this->deviceManager.initialize(std::make_unique<opengl::Device>(std::move(oglContext)));

    this->activeCommandPool = getDevice().createCommandPool({});

    this->initialized = true;
}

void Renderer::begin()
{
    activeCommandBuffer = activeCommandPool->acquireGraphicsCommandBuffer({});

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
    activeCommandBuffer = activeCommandPool->acquireGraphicsCommandBuffer({});

    // create default depth texture
    auto depthAttachment = getDevice().createTexture(TextureDesc::new2D(
            TextureFormat::Z_UNorm24,
            renderTarget.colorTexture->getWidth(),
            renderTarget.colorTexture->getHeight(),
            TextureDesc::TextureUsageBits::Attachment | TextureDesc::TextureUsageBits::Sampled));

    activeFramebuffer = getDevice().createFramebuffer({
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
    auto pipelineDesc = renderable.buildGraphicsPipelineDesc();
    auto pipeline = acquireGraphicsPipeline(pipelineDesc);
    renderable.injectGraphicsPipeline(pipeline);
    renderable.draw(getDevice(), *activeCommandBuffer);
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
    return deviceManager.getDevice();
}
void Renderer::bindViewport(const Viewport& viewport)
{
    this->activeCommandBuffer->bindViewport(viewport);
}

std::shared_ptr<IGraphicsPipeline> Renderer::acquireGraphicsPipeline(const GraphicsPipelineDesc& desc)
{
    auto hash = GraphicsPipelineDescHash{}(desc);
    auto it = graphicsPipelines.find(hash);
    if (it != graphicsPipelines.end())
    {
        return it->second;
    }
    else
    {
        auto pipeline = getDevice().createGraphicsPipeline(desc);
        graphicsPipelines[hash] = pipeline;
        std::cout << "Created new pipeline: " << hash << std::endl;
        return pipeline;
    }
}

}// namespace graphics
