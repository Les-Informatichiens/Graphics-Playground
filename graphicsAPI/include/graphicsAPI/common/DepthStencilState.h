//
// Created by Jonathan Richard on 2024-02-08.
//

#pragma once

#include <cstdint>

enum class CompareOp : uint8_t
{
    Never,
    Less,
    Equal,
    LessOrEqual,
    Greater,
    NotEqual,
    GreaterOrEqual,
    Always
};

enum class StencilOp : uint8_t
{
    Keep,
    Zero,
    Replace,
    IncrementClamp,
    DecrementClamp,
    Invert,
    IncrementWrap,
    DecrementWrap
};

struct DepthStateDesc {
    bool writeEnabled = true;
    CompareOp compareOp = CompareOp::Less;
};

struct StencilStateDesc {
    uint8_t readMask = 0xFF;
    uint8_t writeMask = 0xFF;
    CompareOp compareOp = CompareOp::Always;
    StencilOp failOp = StencilOp::Keep;
    StencilOp depthFailOp = StencilOp::Keep;
    StencilOp passOp = StencilOp::Keep;
};

struct DepthStencilStateDesc
{
    DepthStateDesc depth;
    StencilStateDesc stencilFront;
    StencilStateDesc stencilBack;
};

class IDepthStencilState
{
public:
    virtual ~IDepthStencilState() = default;
};
