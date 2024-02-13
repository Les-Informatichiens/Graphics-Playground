//
// Created by Jonathan Richard on 2024-02-04.
//

#include "graphicsAPI/common/VertexInputState.h"

static uint32_t getVertexAttributeFormatSize(VertexAttributeFormat format)
{
    /*
     *  * Float = 0,
Float2,
Float3,
Float4,

    Byte1,
            Byte2,
            Byte3,
            Byte4,

            UByte1,
            UByte2,
            UByte3,
            UByte4,

            Short1,
            Short2,
            Short3,
            Short4,

            UShort1,
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

            Int1,
            Int2,
            Int3,
            Int4,

            UInt1,
            UInt2,
            UInt3,
            UInt4,

            // packed formats
            HalfFloat1,
            HalfFloat2,
            HalfFloat3,
            HalfFloat4,
            */
    switch (format)
    {
        case VertexAttributeFormat::Float:
            return 4;
        case VertexAttributeFormat::Float2:
            return 8;
        case VertexAttributeFormat::Float3:
            return 12;
        case VertexAttributeFormat::Float4:
            return 16;
        case VertexAttributeFormat::Byte:
            return 1;
        case VertexAttributeFormat::Byte2:
            return 2;
        case VertexAttributeFormat::Byte3:
            return 3;
        case VertexAttributeFormat::Byte4:
            return 4;
        case VertexAttributeFormat::UByte:
            return 1;
        case VertexAttributeFormat::UByte2:
            return 2;
        case VertexAttributeFormat::UByte3:
            return 3;
        case VertexAttributeFormat::UByte4:
            return 4;
        case VertexAttributeFormat::Short:
            return 2;
        case VertexAttributeFormat::Short2:
            return 4;
        case VertexAttributeFormat::Short3:
            return 6;
        case VertexAttributeFormat::Short4:
            return 8;
        case VertexAttributeFormat::UShort:
            return 2;
        case VertexAttributeFormat::UShort2:
            return 4;
        case VertexAttributeFormat::UShort3:
            return 6;
        case VertexAttributeFormat::UShort4:
            return 8;
        case VertexAttributeFormat::Byte2Norm:
            return 2;
        case VertexAttributeFormat::Byte4Norm:
            return 4;
        case VertexAttributeFormat::UByte2Norm:
            return 2;
        case VertexAttributeFormat::UByte4Norm:
            return 4;
        case VertexAttributeFormat::Short2Norm:
            return 4;
        case VertexAttributeFormat::Short4Norm:
            return 8;
        case VertexAttributeFormat::UShort2Norm:
            return 4;
        case VertexAttributeFormat::UShort4Norm:
            return 8;
        case VertexAttributeFormat::Int:
            return 4;
        case VertexAttributeFormat::Int2:
            return 8;
        case VertexAttributeFormat::Int3:
            return 12;
        case VertexAttributeFormat::Int4:
            return 16;
        case VertexAttributeFormat::UInt:
            return 4;
        case VertexAttributeFormat::UInt2:
            return 8;
        case VertexAttributeFormat::UInt3:
            return 12;
        case VertexAttributeFormat::UInt4:
            return 16;
        case VertexAttributeFormat::HalfFloat:
            return 2;
        case VertexAttributeFormat::HalfFloat2:
            return 4;
        case VertexAttributeFormat::HalfFloat3:
            return 6;
        case VertexAttributeFormat::HalfFloat4:
            return 8;
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
