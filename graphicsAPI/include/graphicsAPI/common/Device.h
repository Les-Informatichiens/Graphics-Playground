//
// Created by Jonathan Richard on 2024-01-31.
//

#pragma once

#include "Buffer.h"
#include "CommandPool.h"
#include "DepthStencilState.h"
#include "DeviceFeatures.h"
#include "Framebuffer.h"
#include "GraphicsPipeline.h"
#include "PlatformDevice.h"
#include "SamplerState.h"
#include "ShaderModule.h"
#include "Texture.h"
#include <memory>

class IDevice : public ICapabilities
{
public:
    virtual std::shared_ptr<ICommandPool> createCommandPool(const CommandPoolDesc& desc) = 0;
    virtual std::unique_ptr<IBuffer> createBuffer(const BufferDesc& desc) = 0;
    virtual std::shared_ptr<IShaderModule> createShaderModule(const ShaderModuleDesc& desc) = 0;
    virtual std::shared_ptr<IPipelineShaderStages> createPipelineShaderStages(const PipelineShaderStagesDesc& desc) = 0;
    virtual std::shared_ptr<IVertexInputState> createVertexInputState(const VertexInputStateDesc& desc) = 0;
    virtual std::shared_ptr<IDepthStencilState> createDepthStencilState(const DepthStencilStateDesc& desc) = 0;
    virtual std::shared_ptr<ISamplerState> createSamplerState(const SamplerStateDesc& desc) = 0;

//    virtual std::shared_ptr<IRenderPass> createRenderPass(const RenderPassDesc& desc) = 0;
    virtual std::shared_ptr<IGraphicsPipeline> createGraphicsPipeline(const GraphicsPipelineDesc& desc) = 0;

    virtual std::shared_ptr<IFramebuffer> createFramebuffer(const FramebufferDesc& desc) = 0;
    virtual std::shared_ptr<ITexture> createTexture(const TextureDesc& desc) = 0;

    template<typename T, typename = std::enable_if_t<std::is_base_of<IPlatformDevice, T>::value>>
    T* getPlatformDevice() noexcept {
        return const_cast<T*>(static_cast<const IDevice*>(this)->getPlatformDevice<T>());
    }

};
