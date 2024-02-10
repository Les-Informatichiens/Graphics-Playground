//
// Created by Jonathan Richard on 2024-02-04.
//

#include "VertexInputState.h"
#include <stdexcept>

namespace opengl {

static void toOGLAttribute(const VertexInputAttributeDesc& attrib,
                           GLint& numComponents,
                           GLenum& componentType,
                           GLboolean& normalized) {
    switch (attrib.format) {
        case VertexAttributeFormat::Float:
            numComponents = 1;
            componentType = GL_FLOAT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::Float2:
            numComponents = 2;
            componentType = GL_FLOAT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::Float3:
            numComponents = 3;
            componentType = GL_FLOAT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::Float4:
            numComponents = 4;
            componentType = GL_FLOAT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::Byte:
            numComponents = 1;
            componentType = GL_BYTE;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::Byte2:
            numComponents = 2;
            componentType = GL_BYTE;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::Byte3:
            numComponents = 3;
            componentType = GL_BYTE;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::Byte4:
            numComponents = 4;
            componentType = GL_BYTE;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::UByte:
            numComponents = 1;
            componentType = GL_UNSIGNED_BYTE;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::UByte2:
            numComponents = 2;
            componentType = GL_UNSIGNED_BYTE;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::UByte3:
            numComponents = 3;
            componentType = GL_UNSIGNED_BYTE;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::UByte4:
            numComponents = 4;
            componentType = GL_UNSIGNED_BYTE;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::Short:
            numComponents = 1;
            componentType = GL_SHORT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::Short2:
            numComponents = 2;
            componentType = GL_SHORT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::Short3:
            numComponents = 3;
            componentType = GL_SHORT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::Short4:
            numComponents = 4;
            componentType = GL_SHORT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::UShort:
            numComponents = 1;
            componentType = GL_UNSIGNED_SHORT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::UShort2:
            numComponents = 2;
            componentType = GL_UNSIGNED_SHORT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::UShort3:
            numComponents = 3;
            componentType = GL_UNSIGNED_SHORT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::UShort4:
            numComponents = 4;
            componentType = GL_UNSIGNED_SHORT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::Byte2Norm:
            numComponents = 2;
            componentType = GL_BYTE;
            normalized = GL_TRUE;
            break;
        case VertexAttributeFormat::Byte4Norm:
            numComponents = 4;
            componentType = GL_BYTE;
            normalized = GL_TRUE;
            break;
        case VertexAttributeFormat::UByte2Norm:
            numComponents = 2;
            componentType = GL_UNSIGNED_BYTE;
            normalized = GL_TRUE;
            break;
        case VertexAttributeFormat::UByte4Norm:
            numComponents = 4;
            componentType = GL_UNSIGNED_BYTE;
            normalized = GL_TRUE;
            break;
        case VertexAttributeFormat::Short2Norm:
            numComponents = 2;
            componentType = GL_SHORT;
            normalized = GL_TRUE;
            break;
        case VertexAttributeFormat::Short4Norm:
            numComponents = 4;
            componentType = GL_SHORT;
            normalized = GL_TRUE;
            break;
        case VertexAttributeFormat::UShort2Norm:
            numComponents = 2;
            componentType = GL_UNSIGNED_SHORT;
            normalized = GL_TRUE;
            break;
        case VertexAttributeFormat::UShort4Norm:
            numComponents = 4;
            componentType = GL_UNSIGNED_SHORT;
            normalized = GL_TRUE;
            break;
        case VertexAttributeFormat::Int:
            numComponents = 1;
            componentType = GL_INT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::Int2:
            numComponents = 2;
            componentType = GL_INT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::Int3:
            numComponents = 3;
            componentType = GL_INT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::Int4:
            numComponents = 4;
            componentType = GL_INT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::UInt:
            numComponents = 1;
            componentType = GL_UNSIGNED_INT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::UInt2:
            numComponents = 2;
            componentType = GL_UNSIGNED_INT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::UInt3:
            numComponents = 3;
            componentType = GL_UNSIGNED_INT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::UInt4:
            numComponents = 4;
            componentType = GL_UNSIGNED_INT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::HalfFloat:
            numComponents = 1;
            componentType = GL_HALF_FLOAT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::HalfFloat2:
            numComponents = 2;
            componentType = GL_HALF_FLOAT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::HalfFloat3:
            numComponents = 3;
            componentType = GL_HALF_FLOAT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::HalfFloat4:
            numComponents = 4;
            componentType = GL_HALF_FLOAT;
            normalized = GL_FALSE;
            break;
        default:
            throw std::runtime_error("Invalid vertex attribute format");
    }
}

VertexInputState::VertexInputState(const VertexInputStateDesc& desc)
{
    this->populateBufferAttributes(desc);
}

const std::vector<OpenGLAttributeDesc>& VertexInputState::getBufferAttributes(size_t bufferIndex) const
{
    auto it = bufferAttribMap.find(bufferIndex);
    if (it == bufferAttribMap.end())
    {
        throw std::runtime_error("Buffer index not found in vertex input state");
    }
    return it->second;
}

void VertexInputState::populateBufferAttributes(const VertexInputStateDesc& desc)
{
    const std::vector<VertexInputBindingDesc>& bindingDescs = desc.vertexBindingDescriptions;
    const std::vector<VertexInputAttributeDesc>& attributeDescs = desc.vertexAttributeDescriptions;

    for (const auto& bindingDesc : bindingDescs)
    {
        bufferAttribMap[bindingDesc.binding] = std::vector<OpenGLAttributeDesc>();
    }

    for (const auto& attributeDesc : attributeDescs)
    {
        OpenGLAttributeDesc attribDesc;

        attribDesc.location = attributeDesc.location; // todo: remove when we have a better way to query the location of the attribute in the shader program
        attribDesc.name = attributeDesc.name;
        attribDesc.stride = bindingDescs[attributeDesc.binding].stride;
        attribDesc.offset = attributeDesc.offset;

        toOGLAttribute(attributeDesc, attribDesc.numComponents, attribDesc.type, attribDesc.normalized);

        bufferAttribMap[attributeDesc.binding].push_back(attribDesc);
    }
}

}// namespace opengl
