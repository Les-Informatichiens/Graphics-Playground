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
    FrontFace frontFace = FrontFace::Clockwise;
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

class IGraphicsPipeline
{
public:
    virtual ~IGraphicsPipeline() = default;

    [[nodiscard]] virtual const GraphicsPipelineDesc& getDesc() const = 0;
};