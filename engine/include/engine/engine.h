//
// Created by Jonathan Richard on 2024-01-29.
//
#pragma once
#include "graphicsAPI/common/Device.h"
#include <memory>
#include <utility>

class engine
{
public:

    explicit engine(IDevice& graphicsDevice);;

    void update(float dt);
    void render();

private:
    void createShaderStages();
    void createOffscreenFramebuffer(uint32_t width, uint32_t height);

private:

    std::shared_ptr<IShaderModule> vs;
    std::shared_ptr<IShaderModule> fs;
    std::shared_ptr<IPipelineShaderStages> shaderStages;

    std::shared_ptr<IGraphicsPipeline> pipeline;

    std::shared_ptr<IFramebuffer> fbOffscreen;

    std::shared_ptr<ISamplerState> sampler;

    IDevice& device;
};