//
// Created by Jonathan Richard on 2024-02-04.
//

#include "graphicsAPI/common/VertexInputState.h"

static uint32_t getVertexAttributeFormatSize(VertexAttributeFormat format)
{
    switch (format)
    {
        case VertexAttributeFormat::FLOAT:
            return 4;
        case VertexAttributeFormat::FLOAT2:
            return 8;
        case VertexAttributeFormat::FLOAT3:
            return 12;
        case VertexAttributeFormat::FLOAT4:
            return 16;
        case VertexAttributeFormat::INT:
            return 4;
        case VertexAttributeFormat::INT2:
            return 8;
        case VertexAttributeFormat::INT3:
            return 12;
        case VertexAttributeFormat::INT4:
            return 16;
        case VertexAttributeFormat::UINT:
            return 4;
        case VertexAttributeFormat::UINT2:
            return 8;
        case VertexAttributeFormat::UINT3:
            return 12;
        case VertexAttributeFormat::UINT4:
            return 16;
    }
    return 0;
}

VertexInputStateDescBuilder& VertexInputStateDescBuilder::beginBinding(uint32_t binding)
{
    currentBindingDesc = {
        .binding = binding,
        .stride = 0,
    };
    bindingStarted = true;
    return *this;
}
//{
//    currentBindingDesc = {
//        .binding = binding,
//        .stride = 0,
//    };
//    bindingStarted = true;
//    return *this;
//}

VertexInputStateDescBuilder& VertexInputStateDescBuilder::addVertexAttribute(VertexAttributeFormat format, const std::string& name, uint32_t location)
{
    if (!bindingStarted)
    {
        return *this;
    }

    VertexInputAttributeDesc attributeDesc;
    attributeDesc.location = location;
    attributeDesc.binding = currentBindingDesc.binding;
    attributeDesc.format = format;
    attributeDesc.name = name;
    attributeDesc.size = getVertexAttributeFormatSize(format);
    attributeDesc.offset = currentBindingAttribOffset;
    currentBindingAttribOffset += attributeDesc.size;
//    ++currentBindingAttribLocation;

    currentBindingDesc.stride += attributeDesc.size;

    vertexInputStateDesc.vertexAttributeDescriptions.push_back(std::move(attributeDesc));

    return *this;
}

VertexInputStateDesc VertexInputStateDescBuilder::build() const
{
    return vertexInputStateDesc;
}

VertexInputStateDescBuilder& VertexInputStateDescBuilder::endBinding()
{
    if (!bindingStarted)
    {
        return *this;
    }

    bindingStarted = false;
    vertexInputStateDesc.vertexBindingDescriptions.push_back(currentBindingDesc);

//    currentBindingAttribLocation = 0;
    currentBindingAttribOffset = 0;

    return *this;
}
