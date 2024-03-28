//
// Created by Jonathan Richard on 2024-03-06.
//

#pragma once

#include <utility>
#include <algorithm>

#include "ShaderProgram.h"
#include "graphicsAPI/common/ShaderStage.h"

namespace graphics {


enum class DepthTestConfig {
    Disable,
    Enable,
    EnableNoWrite,
};

/// Aggregates all blend mode related configurations.
class BlendMode {
public:
    BlendFactor srcRGB;
    BlendFactor dstRGB;
    BlendOp opRGB;
    BlendFactor srcAlpha;
    BlendFactor dstAlpha;
    BlendOp opAlpha;

    BlendMode(BlendFactor src, BlendFactor dst) :
                                                            BlendMode(src, dst, BlendOp::Add, src, dst, BlendOp::Add) {}
    BlendMode(BlendFactor srcRGB,
              BlendFactor dstRGB,
              BlendOp opRGB,
              BlendFactor srcAlpha,
              BlendFactor dstAlpha,
              BlendOp opAlpha) :
                                      srcRGB(srcRGB),
                                      dstRGB(dstRGB),
                                      opRGB(opRGB),
                                      srcAlpha(srcAlpha),
                                      dstAlpha(dstAlpha),
                                      opAlpha(opAlpha) {}

    static BlendMode Opaque() {
        return {BlendFactor::One, BlendFactor::Zero};
    }
    static BlendMode Translucent() {
        return {BlendFactor::SrcAlpha,
                         BlendFactor::OneMinusSrcAlpha,
                         BlendOp::Add,
                         BlendFactor::One,
                         BlendFactor::OneMinusSrcAlpha,
                         BlendOp::Add};
    }
    static BlendMode Additive() {
        return {BlendFactor::SrcAlpha, BlendFactor::One};
    }
    static BlendMode Premultiplied() {
        return {BlendFactor::One, BlendFactor::OneMinusSrcAlpha};
    }

    bool operator==(const BlendMode& other) const {
        return srcRGB == other.srcRGB && dstRGB == other.dstRGB && opRGB == other.opRGB &&
               srcAlpha == other.srcAlpha && dstAlpha == other.dstAlpha && opAlpha == other.opAlpha;
    }
};

class Material
{
    struct UniformBufferDesc
    {
        size_t size = 0;
        std::vector<uint8_t> data;
        std::shared_ptr<IBuffer> buffer;
        size_t bindingPoint = 0;
    };

    struct TextureSamplerDesc
    {
        size_t index = 0;
        std::shared_ptr<ITexture> texture;
        std::shared_ptr<ISamplerState> samplerState;
    };

    struct DepthStencilDesc
    {
        DepthStencilStateDesc desc;
        std::shared_ptr<IDepthStencilState> depthStencilState;
        bool isDirty = false;
    };

public:
    explicit Material(IDevice& device, std::shared_ptr<graphics::ShaderProgram> shaderProgram)
        : shaderProgram(std::move(shaderProgram)) {}

    void setShaderProgram(std::shared_ptr<graphics::ShaderProgram> shaderProgram_)
    {
        this->shaderProgram = std::move(shaderProgram_);
    }

    [[nodiscard]] std::shared_ptr<graphics::ShaderProgram> getShaderProgram() const
    {
        return shaderProgram;
    }

    void setDepthTestConfig(DepthTestConfig config)
    {
        depthTestConfig = config;

        depthStencilDesc.desc.depth.compareOp = (depthTestConfig != DepthTestConfig::Disable)
            ? CompareOp::Less
            : CompareOp::Always;

        depthStencilDesc.desc.depth.writeEnabled = (depthTestConfig == DepthTestConfig::Enable);

        depthStencilDesc.isDirty = true;
    }

    [[nodiscard]] DepthTestConfig getDepthTestConfig() const
    {
        return depthTestConfig;
    }

    void setBlendMode(BlendMode blendMode_)
    {
        blendMode = blendMode_;
    }

    [[nodiscard]] BlendMode getBlendMode() const
    {
        return blendMode;
    }

    void setCullMode(CullMode cullMode_)
    {
        cullMode = cullMode_;
    }

    [[nodiscard]] CullMode getCullMode() const
    {
        return cullMode;
    }

    // Add methods to set and get other material properties (color, texture, etc.)
    void setUniformBytes(const std::string& name, const void* data, size_t size, size_t bindingPoint = 0)
    {
        // if the uniform buffer is not found or the size is too small, put the data in a temporary UniformBufferDesc.
        // we cant create the buffer yet because we don't have the device.
        // we will create the buffer when we bind the material to the command buffer.
        auto it = uniformBuffers.find(name);
        if (it == uniformBuffers.end() || it->second.size < size)
        {
            // create a new buffer desc and copy the data into a temporary vector
            auto tempData = std::vector<uint8_t>(size);
            std::copy(static_cast<const uint8_t*>(data), static_cast<const uint8_t*>(data) + size, tempData.begin());
            uniformBuffers[name] = {size, tempData, nullptr, bindingPoint};
//            std::cout << "Created new uniform buffer: " << name << std::endl;
        }
        else
        {
            it->second.buffer->data(data, size, 0);
        }
    }

    void setTextureSampler(const std::string& name, std::shared_ptr<ITexture> texture, std::shared_ptr<ISamplerState> samplerState, size_t index = 0)
    {
        textureSamplers[name] = {index, std::move(texture), std::move(samplerState)};
    }

    void bind(IDevice& device, IGraphicsCommandBuffer& commandBuffer)
    {
        // build the pipeline if it's not built yet
//        build(device);

        // bind the shader program
//        commandBuffer.bindGraphicsPipeline(cachedPipeline);

        // bind the uniform buffers
        for (auto& [name, bufferDesc] : uniformBuffers)
        {
            if (!bufferDesc.buffer)
            {
                auto newBufferDesc = BufferDesc {
                    .type = BufferDesc::BufferTypeBits::Uniform,
                    .data = bufferDesc.data.data(),
                    .size = static_cast<uint32_t>(bufferDesc.size),
                    .storage = ResourceStorage::Shared,
                };
                bufferDesc.buffer = device.createBuffer(newBufferDesc);
//                std::cout << "Created new uniform buffer: " << name << std::endl;

            }
            commandBuffer.bindBuffer(bufferDesc.bindingPoint, bufferDesc.buffer, 0);
        }

        // bind the texture samplers
        for (const auto& [name, textureSampler] : textureSamplers)
        {
            commandBuffer.bindTexture(textureSampler.index, BindTarget::BindTarget_Fragment, textureSampler.texture);
            commandBuffer.bindSamplerState(textureSampler.index, BindTarget::BindTarget_Fragment, textureSampler.samplerState);
        }

        // bind the depth stencil state
        if (depthStencilDesc.isDirty || !depthStencilDesc.depthStencilState)
        {
            depthStencilDesc.depthStencilState = device.createDepthStencilState(depthStencilDesc.desc);
            depthStencilDesc.isDirty = false;
        }
        commandBuffer.bindDepthStencilState(depthStencilDesc.depthStencilState);
    }

    void preparePipelineDesc(GraphicsPipelineDesc& desc) const
    {
        if (!desc.colorBlendAttachmentStates.empty())
        {
            auto& colorBlendAttachmentState = desc.colorBlendAttachmentStates[0];
            colorBlendAttachmentState.blendEnabled = (blendMode != BlendMode::Opaque());
            colorBlendAttachmentState.srcColorBlendFactor = blendMode.srcRGB;
            colorBlendAttachmentState.dstColorBlendFactor = blendMode.dstRGB;
            colorBlendAttachmentState.colorBlendOp = blendMode.opRGB;
            colorBlendAttachmentState.srcAlphaBlendFactor = blendMode.srcAlpha;
            colorBlendAttachmentState.dstAlphaBlendFactor = blendMode.dstAlpha;
            colorBlendAttachmentState.alphaBlendOp = blendMode.opAlpha;
        }

        auto& rasterizationState = desc.rasterizationState;
        rasterizationState.cullMode = cullMode;

        for (const auto& [name, textureSampler] : textureSamplers)
        {
            desc.fragmentUnitSamplerMap[textureSampler.index] = name;
        }

        shaderProgram->preparePipelineDesc(desc);
    }
//
//    void build(IDevice& device)
//    {
//        GraphicsPipelineDesc desc;
//        preparePipelineDesc(desc);
//        if (GraphicsPipelineDescHash{}(desc) != GraphicsPipelineDescHash{}(pipelineDesc))
//        {
//            pipelineDesc = desc;
//            cachedPipeline = device.createGraphicsPipeline(pipelineDesc);
//        }
//    }

private:
    std::string name = "default";

    BlendMode blendMode = BlendMode::Opaque();
    CullMode cullMode = CullMode::None;
    DepthTestConfig depthTestConfig = DepthTestConfig::Enable;

    std::shared_ptr<graphics::ShaderProgram> shaderProgram;

    std::unordered_map<std::string, UniformBufferDesc> uniformBuffers;
    std::unordered_map<std::string, TextureSamplerDesc> textureSamplers;

    DepthStencilDesc depthStencilDesc = {};

    //Pipeline
//    GraphicsPipelineDesc pipelineDesc;
//    std::shared_ptr<IGraphicsPipeline> cachedPipeline;

};

}// namespace graphics
