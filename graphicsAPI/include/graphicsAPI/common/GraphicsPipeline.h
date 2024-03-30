//
// Created by Jonathan Richard on 2024-02-01.
//

#pragma once

#include "ShaderStage.h"
#include "VertexInputState.h"
#include <memory>

enum class BlendOp : uint32_t
{
    Add = 0,
    Subtract = 1,
    ReverseSubtract = 2,
    Min = 3,
    Max = 4
};

enum class BlendFactor : uint32_t
{
    Zero = 0,
    One = 1,
    SrcColor = 2,
    OneMinusSrcColor = 3,
    DstColor = 4,
    OneMinusDstColor = 5,
    SrcAlpha = 6,
    OneMinusSrcAlpha = 7,
    DstAlpha = 8,
    OneMinusDstAlpha = 9,
    ConstantColor = 10,
    OneMinusConstantColor = 11,
    ConstantAlpha = 12,
    OneMinusConstantAlpha = 13,
    SrcAlphaSaturate = 14,
    Src1Color = 15,
    OneMinusSrc1Color = 16,
    Src1Alpha = 17,
    OneMinusSrc1Alpha = 18
};

enum ColorWriteMask : uint8_t
{
    R = 0x1,
    G = 0x2,
    B = 0x4,
    A = 0x8,
    All = R | G | B | A
};

enum class CullMode : uint8_t
{
    None = 0,
    Front = 1,
    Back = 2
};

enum class FrontFace : uint8_t
{
    Clockwise = 0,
    CounterClockwise = 1
};

enum class PolygonFillMode : uint8_t
{
    Fill = 0,
    Line = 1,
};

struct ColorBlendAttachmentStateDesc
{
    bool blendEnabled = false;
    BlendFactor srcColorBlendFactor = BlendFactor::One;
    BlendFactor dstColorBlendFactor = BlendFactor::Zero;
    BlendOp colorBlendOp = BlendOp::Add;
    BlendFactor srcAlphaBlendFactor = BlendFactor::One;
    BlendFactor dstAlphaBlendFactor = BlendFactor::Zero;
    BlendOp alphaBlendOp = BlendOp::Add;
    ColorWriteMask colorWriteMask = ColorWriteMask::All;
};

struct RasterizationStateDesc
{
    CullMode cullMode = CullMode::None;
    FrontFace frontFace = FrontFace::CounterClockwise;
    PolygonFillMode polygonFillMode = PolygonFillMode::Fill;
};

struct GraphicsPipelineDesc
{
    std::shared_ptr<IPipelineShaderStages> shaderStages;
    std::shared_ptr<IVertexInputState> vertexInputState;

    std::vector<ColorBlendAttachmentStateDesc> colorBlendAttachmentStates;

    RasterizationStateDesc rasterizationState;

    /*
   * GL Only: Mapping of Texture Unit <-> Sampler Name
   * Texture unit should be < MAX_TEXTURE_SAMPLERS
   */
    std::unordered_map<size_t, std::string> vertexUnitSamplerMap;
    std::unordered_map<size_t, std::string> fragmentUnitSamplerMap;
};

struct ColorBlendAttachmentStateDescHash
{
    size_t operator()(const ColorBlendAttachmentStateDesc& desc) const
    {
        size_t hash = 0;
        hash_combine(hash, std::hash<bool>{}(desc.blendEnabled));
        hash_combine(hash, std::hash<BlendFactor>{}(desc.srcColorBlendFactor));
        hash_combine(hash, std::hash<BlendFactor>{}(desc.dstColorBlendFactor));
        hash_combine(hash, std::hash<BlendOp>{}(desc.colorBlendOp));
        hash_combine(hash, std::hash<BlendFactor>{}(desc.srcAlphaBlendFactor));
        hash_combine(hash, std::hash<BlendFactor>{}(desc.dstAlphaBlendFactor));
        hash_combine(hash, std::hash<BlendOp>{}(desc.alphaBlendOp));
        hash_combine(hash, std::hash<ColorWriteMask>{}(desc.colorWriteMask));
        return hash;
    }
};

struct RasterizationStateDescHash
{
    size_t operator()(const RasterizationStateDesc& desc) const
    {
        size_t hash = 0;
        hash_combine(hash, std::hash<int>{}(EnumToValue(desc.cullMode)));
        hash_combine(hash, std::hash<int>{}(EnumToValue(desc.frontFace)));
        hash_combine(hash, std::hash<int>{}(EnumToValue(desc.polygonFillMode)));
        return hash;
    }
};

struct GraphicsPipelineDescHash
{
    size_t operator()(const GraphicsPipelineDesc& desc) const
    {
        size_t hash = 0;
        hash_combine(hash, std::hash<std::shared_ptr<IPipelineShaderStages>>{}(desc.shaderStages));
        hash_combine(hash, std::hash<std::shared_ptr<IVertexInputState>>{}(desc.vertexInputState));
        for (const auto& colorBlendAttachmentState : desc.colorBlendAttachmentStates)
        {
            hash_combine(hash, ColorBlendAttachmentStateDescHash{}(colorBlendAttachmentState));
        }
        hash_combine(hash, RasterizationStateDescHash{}(desc.rasterizationState));
        for (const auto& [unit, sampler] : desc.vertexUnitSamplerMap)
        {
            hash_combine(hash, unit);
            hash_combine(hash, sampler);
        }
        for (const auto& [unit, sampler] : desc.fragmentUnitSamplerMap)
        {
            hash_combine(hash, unit);
            hash_combine(hash, sampler);
        }
//
//        // print all individual hashes
//        std::cout << "hash: " << hash << std::endl;
//        std::cout << "ShaderStages: " << std::hash<std::shared_ptr<IPipelineShaderStages>>{}(desc.shaderStages) << std::endl;
//        std::cout << "VertexInputState: " << std::hash<std::shared_ptr<IVertexInputState>>{}(desc.vertexInputState) << std::endl;
//        for (const auto& colorBlendAttachmentState : desc.colorBlendAttachmentStates)
//        {
//            std::cout << "ColorBlendAttachmentState: " << ColorBlendAttachmentStateDescHash{}(colorBlendAttachmentState) << std::endl;
//        }
//        std::cout << "RasterizationState: " << RasterizationStateDescHash{}(desc.rasterizationState) << std::endl;
//        for (const auto& [unit, sampler] : desc.vertexUnitSamplerMap)
//        {
//            std::cout << "VertexUnitSamplerMap: " << unit << " " << sampler << std::endl;
//        }
//        for (const auto& [unit, sampler] : desc.fragmentUnitSamplerMap)
//        {
//            std::cout << "FragmentUnitSamplerMap: " << unit << " " << sampler << std::endl;
//        }
//


        return hash;
    }
};

class IGraphicsPipeline
{
public:
    virtual ~IGraphicsPipeline() = default;

    [[nodiscard]] virtual const GraphicsPipelineDesc& getDesc() const = 0;
};