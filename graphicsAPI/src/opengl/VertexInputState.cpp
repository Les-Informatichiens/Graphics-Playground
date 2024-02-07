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
        case VertexAttributeFormat::FLOAT:
            numComponents = 1;
            componentType = GL_FLOAT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::FLOAT2:
            numComponents = 2;
            componentType = GL_FLOAT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::FLOAT3:
            numComponents = 3;
            componentType = GL_FLOAT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::FLOAT4:
            numComponents = 4;
            componentType = GL_FLOAT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::INT:
            numComponents = 1;
            componentType = GL_INT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::INT2:
            numComponents = 2;
            componentType = GL_INT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::INT3:
            numComponents = 3;
            componentType = GL_INT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::INT4:
            numComponents = 4;
            componentType = GL_INT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::UINT:
            numComponents = 1;
            componentType = GL_UNSIGNED_INT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::UINT2:
            numComponents = 2;
            componentType = GL_UNSIGNED_INT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::UINT3:
            numComponents = 3;
            componentType = GL_UNSIGNED_INT;
            normalized = GL_FALSE;
            break;
        case VertexAttributeFormat::UINT4:
            numComponents = 4;
            componentType = GL_UNSIGNED_INT;
            normalized = GL_FALSE;
            break;
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
