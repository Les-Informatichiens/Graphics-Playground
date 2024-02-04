//
// Created by Jonathan Richard on 2024-02-03.
//

#pragma once

#include "Context.h"
#include "PlatformDevice.h"
#include "graphicsAPI/common/Device.h"
#include <spirv_glsl.hpp>

namespace opengl
{

class Device : public IDevice
{

public:
    std::shared_ptr<ICommandPool> createCommandPool(const CommandPoolDesc& desc) override;
    std::unique_ptr<IBuffer> createBuffer(const BufferDesc& desc) override;
    std::shared_ptr<IShaderModule> createShaderModule(const ShaderModuleDesc& desc) override;
    std::shared_ptr<IPipelineShaderStages> createPipelineShaderStages(const PipelineShaderStagesDesc& desc) override;
    std::shared_ptr<IGraphicsPipeline> createGraphicsPipeline(const GraphicsPipelineDesc& desc) override;
    std::shared_ptr<IFramebuffer> createFramebuffer(const FramebufferDesc& desc) override;
    std::shared_ptr<ITexture> createTexture(const TextureDesc& desc) override;

    [[nodiscard]] ShaderVersion getShaderVersion() const override
    {
        return {
            .family = ShaderFamily::GLSL,
            .major = 4,
            .minor = 6
        };
    }

    [[nodiscard]] bool hasFeature(DeviceFeatures feature) const override;
    [[nodiscard]] TextureFormatCapabilities getTextureFormatCapabilities(TextureFormat format) const override;

private:
    PlatformDevice platformDevice;
    Context context;
};

}
