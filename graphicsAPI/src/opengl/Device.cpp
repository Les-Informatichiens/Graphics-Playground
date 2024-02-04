//
// Created by Jonathan Richard on 2024-02-03.
//

#include "graphicsAPI/opengl/Device.h"
#include "GraphicsPipeline.h"
#include "ShaderModule.h"
#include "ShaderStage.h"

namespace opengl {

std::shared_ptr<ICommandPool> Device::createCommandPool(const CommandPoolDesc& desc)
{
    return std::shared_ptr<ICommandPool>();
}

std::unique_ptr<IBuffer> Device::createBuffer(const BufferDesc& desc)
{
    return std::unique_ptr<IBuffer>();
}

std::shared_ptr<IShaderModule> Device::createShaderModule(const ShaderModuleDesc& desc)
{
    auto shaderModule = std::make_shared<ShaderModule>(context, desc);

    spirv_cross::CompilerGLSL compiler(reinterpret_cast<const uint32_t*>(desc.code.data()), desc.code.size() / sizeof(uint32_t));
    std::string compiledCode = shaderModule->compileAndParseGLSL(compiler);
    shaderModule->create(compiledCode);

    return shaderModule;
}

std::shared_ptr<IPipelineShaderStages> Device::createPipelineShaderStages(const PipelineShaderStagesDesc& desc)
{
    auto shaderStages = std::make_shared<PipelineShaderStages>(context, desc);
    shaderStages->createProgram();
    return shaderStages;
}

std::shared_ptr<IGraphicsPipeline> Device::createGraphicsPipeline(const GraphicsPipelineDesc& desc)
{
    return std::make_shared<GraphicsPipeline>(context, desc);
}

std::shared_ptr<IFramebuffer> Device::createFramebuffer(const FramebufferDesc& desc)
{
    return std::shared_ptr<IFramebuffer>();
}

std::shared_ptr<ITexture> Device::createTexture(const TextureDesc& desc)
{
    return std::shared_ptr<ITexture>();
}

bool Device::hasFeature(DeviceFeatures feature) const
{
    // unimplemented
    return false;
}

ICapabilities::TextureFormatCapabilities Device::getTextureFormatCapabilities(TextureFormat format) const
{
    // unimplemented
    return 0;
}

}// namespace opengl
