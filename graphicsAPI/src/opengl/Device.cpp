//
// Created by Jonathan Richard on 2024-02-03.
//

#include "graphicsAPI/opengl/Device.h"
#include "CommandPool.h"
#include "GraphicsPipeline.h"
#include "ShaderModule.h"
#include "ShaderStage.h"
#include "VertexInputState.h"
#include "graphicsAPI/opengl/Buffer.h"
#include "shaderc/shaderc.hpp"
#include <spirv_glsl.hpp>

namespace opengl {

Device::Device(std::unique_ptr<Context> context_)
    : context(std::move(context_))
{
    context->init();
}

std::shared_ptr<ICommandPool> Device::createCommandPool(const CommandPoolDesc& desc)
{
    if (!commandPool) {
        commandPool = std::make_shared<CommandPool>(context, desc);
    }
    return commandPool;
}

std::unique_ptr<IBuffer> Device::createBuffer(const BufferDesc& desc)
{
    std::unique_ptr<Buffer> resource;
    auto bufferType = desc.type;

    if ((bufferType & BufferDesc::BufferTypeBits::Index) ||
        (bufferType & BufferDesc::BufferTypeBits::Vertex) ||
        (bufferType & BufferDesc::BufferTypeBits::Storage) ||
        (bufferType & BufferDesc::BufferTypeBits::Uniform))
    {
        resource = std::make_unique<ArrayBuffer>(getContext());
    }
    if (!resource) {
        return nullptr;
    }
    resource->initialize(desc);
    return resource;
}

std::shared_ptr<IShaderModule> Device::createShaderModule(const ShaderModuleDesc& desc)
{
    auto shaderModule = std::make_shared<ShaderModule>(getContext(), desc);

    shaderc::Compiler glslcompiler;
    shaderc::CompileOptions options;
    options.SetVulkanRulesRelaxed(true);
    shaderc::SpvCompilationResult result = glslcompiler.CompileGlslToSpv(desc.code, shaderc_shader_kind::shaderc_glsl_vertex_shader, "main", options);
    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        //handle errors
        throw std::runtime_error("Failed to compile shader : " + result.GetErrorMessage());
    }
    std::vector<uint32_t> vertexSPRV;
    vertexSPRV.assign(result.begin(), result.end());

    //    spirv_cross::CompilerGLSL compiler(reinterpret_cast<const uint32_t*>(desc.code.data()), desc.code.size() / sizeof(uint32_t));
        spirv_cross::CompilerGLSL compiler(vertexSPRV);

    std::string compiledCode = shaderModule->compileAndParseGLSL(compiler);
    shaderModule->create(compiledCode);

    return shaderModule;
}

std::shared_ptr<IPipelineShaderStages> Device::createPipelineShaderStages(const PipelineShaderStagesDesc& desc)
{
    auto shaderStages = std::make_shared<PipelineShaderStages>(getContext(), desc);
    shaderStages->createProgram();
    return shaderStages;
}

std::shared_ptr<IGraphicsPipeline> Device::createGraphicsPipeline(const GraphicsPipelineDesc& desc)
{
    return std::make_shared<GraphicsPipeline>(getContext(), desc);
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

std::shared_ptr<IVertexInputState> Device::createVertexInputState(const VertexInputStateDesc& desc)
{
    auto vertexInputState = std::make_shared<VertexInputState>(desc);
    return vertexInputState;
}

Context& Device::getContext() const
{
    return *context;
}

}// namespace opengl
