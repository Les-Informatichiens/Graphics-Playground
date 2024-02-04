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
    FLOAT,
    FLOAT2,
    FLOAT3,
    FLOAT4,
    INT,
    INT2,
    INT3,
    INT4,
    UINT,
    UINT2,
    UINT3,
    UINT4,
};

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
    VertexAttributeFormat format = VertexAttributeFormat::FLOAT;
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
    VertexInputStateDescBuilder& addVertexAttribute(VertexAttributeFormat format, const std::string& name);
    VertexInputStateDescBuilder& endBinding();
    [[nodiscard]] VertexInputStateDesc build() const;

private:
    VertexInputStateDesc vertexInputStateDesc;
    uint32_t currentBindingAttribOffset = 0;
    uint32_t currentBindingAttribLocation = 0;
    VertexInputBindingDesc currentBindingDesc;
    bool bindingStarted = false;
};

class IVertexInputState
{
public:
    virtual ~IVertexInputState() = default;
    [[nodiscard]] virtual const VertexInputStateDesc& getDesc() const = 0;
};
