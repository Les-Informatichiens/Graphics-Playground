//
// Created by Jonathan Richard on 2024-01-29.
//
#pragma once
#include "graphicsAPI/common/Device.h"
#include <memory>
#include <utility>

class TestRenderPass
{
public:
    TestRenderPass() = default;
    ~TestRenderPass() = default;

    void initialize(IDevice& device);

    void render(IDevice& device);

private:
    void createShaderStages(IDevice& device);
    void createOffscreenFramebuffer(IDevice& device, uint32_t width, uint32_t height);

private:

    std::shared_ptr<ITexture> testLoadedTexture;
    std::shared_ptr<ITexture> testGeneratedTexture;

    std::shared_ptr<IShaderModule> vs;
    std::shared_ptr<IShaderModule> fs;
    std::shared_ptr<IPipelineShaderStages> shaderStages;

    std::shared_ptr<IGraphicsPipeline> pipeline;

    std::shared_ptr<IFramebuffer> fbOffscreen;

    std::shared_ptr<ISamplerState> sampler;
};