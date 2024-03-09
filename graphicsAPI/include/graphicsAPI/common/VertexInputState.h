//
// Created by Jonathan Richard on 2024-02-01.
//

#pragma once

#include "Util.h"
#include <cstdint>
#include <string>
#include <vector>

enum class VertexAttributeFormat
{
    Float = 0,
    Float2,
    Float3,
    Float4,

    Byte,
    Byte2,
    Byte3,
    Byte4,

    UByte,
    UByte2,
    UByte3,
    UByte4,

    Short,
    Short2,
    Short3,
    Short4,

    UShort,
    UShort2,
    UShort3,
    UShort4,

    // Normalized variants
    Byte2Norm,
    Byte4Norm,

    UByte2Norm,
    UByte4Norm,

    Short2Norm,
    Short4Norm,

    UShort2Norm,
    UShort4Norm,

    Int,
    Int2,
    Int3,
    Int4,

    UInt,
    UInt2,
    UInt3,
    UInt4,

    // packed formats
    HalfFloat,
    HalfFloat2,
    HalfFloat3,
    HalfFloat4,
};

uint32_t getVertexAttributeFormatSize(VertexAttributeFormat format);

//enum class VertexInputRate
//{
//    VERTEX,
//    INSTANCE,
//};

//struct VertexInputBindingDesc
//{
//    uint32_t binding = 0; // buffer index
//    uint32_t stride = 0; // size of each vertex in bytes
//    VertexInputRate inputRate = VertexInputRate::VERTEX; // per vertex or per instance
//};

//struct VertexInputAttributeDesc
//{
//    uint32_t location = 0; // attribute index
//    uint32_t binding = 0; // buffer index
//    VertexAttributeFormat format = VertexAttributeFormat::FLOAT;
//    std::string name; // attribute name (only for opengl to query the location of the attribute in the shader program)
//    uint32_t size = 0; // size of the attribute in bytes
//    uint32_t offset = 0; // offset in bytes from the start of the vertex
//};

//struct VertexInputStateDesc
//{
//    uint32_t vertexBindingDescriptionCount = 0;
//    std::array<VertexInputBindingDesc, MAX_VERTEX_INPUT_BINDINGS> vertexBindingDescriptions = {};
//    uint32_t vertexAttributeDescriptionCount = 0;
//    std::array<VertexInputAttributeDesc, MAX_VERTEX_INPUT_ATTRIBUTES> vertexAttributeDescriptions = {};
//};

struct VertexInputBindingDesc
{
    uint32_t binding = 0; // buffer index
    uint32_t stride = 0; // size of each vertex in bytes
    //VertexInputRate inputRate = VertexInputRate::VERTEX; // per vertex or per instance
};

struct VertexInputAttributeDesc
{
    uint32_t location = 0;
    uint32_t binding = 0; // buffer index
    VertexAttributeFormat format = VertexAttributeFormat::Float;
    std::string name; // attribute name (only for opengl to query the location of the attribute in the shader program)
    uint32_t size = 0; // size of the attribute in bytes
    uint32_t offset = 0; // offset in bytes from the start of the vertex
};


struct VertexInputStateDesc
{
    std::vector<VertexInputBindingDesc> vertexBindingDescriptions;
    std::vector<VertexInputAttributeDesc> vertexAttributeDescriptions;
};

class VertexInputStateDescBuilder
{
public:
    VertexInputStateDescBuilder() = default;
    VertexInputStateDescBuilder& beginBinding(uint32_t binding);
    VertexInputStateDescBuilder& addVertexAttribute(VertexAttributeFormat format, const std::string& name, uint32_t location = 0);
    VertexInputStateDescBuilder& endBinding();
    [[nodiscard]] VertexInputStateDesc build() const;

private:
    VertexInputStateDesc vertexInputStateDesc;
    uint32_t currentBindingAttribOffset = 0;
//    uint32_t currentBindingAttribLocation = 0;
    VertexInputBindingDesc currentBindingDesc;
    bool bindingStarted = false;
};

class IVertexInputState
{
public:
    virtual ~IVertexInputState() = default;
};
