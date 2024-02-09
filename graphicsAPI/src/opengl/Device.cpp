//
// Created by Jonathan Richard on 2024-02-03.
//

#include "graphicsAPI/opengl/Device.h"
#include "CommandPool.h"
#include "DepthStencilState.h"
#include "Framebuffer.h"
#include "GraphicsPipeline.h"
#include "Renderbuffer.h"
#include "SamplerState.h"
#include "ShaderModule.h"
#include "ShaderStage.h"
#include "TextureBuffer.h"
#include "VertexInputState.h"
#include "graphicsAPI/opengl/Buffer.h"
#include "shaderc/shaderc.hpp"
#include <spirv_glsl.hpp>

#include <iostream>

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
    auto framebuffer = std::make_shared<Framebuffer>(getContext());
    framebuffer->create(desc);
    return framebuffer;
}

std::shared_ptr<ITexture> Device::createTexture(const TextureDesc& desc)
{
    const auto sanitizedDesc = sanitizeTextureDesc(desc);
    std::unique_ptr<Texture> texture;

    if ((sanitizedDesc.usage & TextureDesc::TextureUsageBits::Sampled) != 0 ||
        (sanitizedDesc.usage & TextureDesc::TextureUsageBits::Storage) != 0) {
        texture = std::make_unique<TextureBuffer>(getContext(), desc.format);
    } else if ((sanitizedDesc.usage & TextureDesc::TextureUsageBits::Attachment) != 0) {
        if (sanitizedDesc.type == TextureType::Texture2D) {
            texture = std::make_unique<Renderbuffer>(getContext(), desc.format);
        } else {
            // Fall back to texture. e.g. TextureType::TwoDArray
            texture = std::make_unique<TextureBuffer>(getContext(), desc.format);
        }
    }

    if (texture != nullptr)
    {
        texture->create(sanitizedDesc, false);
    }

    return texture;
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

std::shared_ptr<IDepthStencilState> Device::createDepthStencilState(const DepthStencilStateDesc& desc)
{
    return std::make_shared<DepthStencilState>(getContext(), desc);
}

std::shared_ptr<ISamplerState> Device::createSamplerState(const SamplerStateDesc& desc)
{
    return std::make_shared<SamplerState>(getContext(), desc);
}

Context& Device::getContext() const
{
    return *context;
}
TextureDesc Device::sanitizeTextureDesc(const TextureDesc& desc) const
{
    TextureDesc sanitized = desc;
    if (desc.width == 0 || desc.height == 0 || desc.depth == 0 || desc.numLayers == 0 ||
        desc.numSamples == 0 || desc.numMipLevels == 0) {
        sanitized.width = std::max(sanitized.width, static_cast<size_t>(1));
        sanitized.height = std::max(sanitized.height, static_cast<size_t>(1));
        sanitized.depth = std::max(sanitized.depth, static_cast<size_t>(1));
        sanitized.numLayers = std::max(sanitized.numLayers, static_cast<size_t>(1));
        sanitized.numSamples = std::max(sanitized.numSamples, static_cast<size_t>(1));
        sanitized.numMipLevels = std::max(sanitized.numMipLevels, static_cast<size_t>(1));
    }

    return sanitized;
}

}// namespace opengl
