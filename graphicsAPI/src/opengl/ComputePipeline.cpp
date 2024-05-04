//
// Created by Jonathan Richard on 2024-03-28.
//

#include "ComputePipeline.h"
#include "Texture.h"
#include "graphicsAPI/opengl/Buffer.h"

namespace opengl {

ComputePipeline::ComputePipeline(Context& context, const ComputePipelineDesc& desc)
    : WithContext(context)
{
    this->initialize(desc);
}

ComputePipeline::~ComputePipeline() = default;

void ComputePipeline::initialize(const ComputePipelineDesc& desc)
{
    if (!desc.shaderStages)
    {
        throw std::runtime_error("ComputePipelineDesc::shaderStages is required");
    }

    if (desc.shaderStages->getType() != ShaderStagesType::Compute)
    {
        throw std::runtime_error("ComputePipelineDesc::shaderStages must be of type Compute");
    }

    auto glShaderStages = std::static_pointer_cast<PipelineShaderStages>(desc.shaderStages);
    shaderStages = glShaderStages;
    reflection = std::make_shared<ComputePipelineReflection>(getContext(), *glShaderStages);

    for (const auto& [texUnit, texName]: desc.imagesMap)
    {
        GLint loc = reflection->getLocation(texName);
        if (loc >= 0)
        {
            GLint unit = 0;
            getContext().getUniformiv(shaderStages->getProgram(), loc, &unit);
            if (unit >= 0)
            {
                imageUnitMap[texUnit] = unit;
            }
            else
            {
                std::cerr << "Image uniform unit (" << texName << ") not found in shader" << std::endl;
            }
        }
        else
        {
            std::cerr << "Image uniform (" << texName << ") not found in shader" << std::endl;
        }
    }

    for (const auto& [texUnit, texName]: desc.texturesMap)
    {
        GLint loc = reflection->getLocation(texName);
        if (loc >= 0)
        {
            GLint unit = 0;
            getContext().getUniformiv(shaderStages->getProgram(), loc, &unit);
            if (unit >= 0)
            {
                textureUnitMap[texUnit] = loc;
            }
            else
            {
                std::cerr << "Texture uniform unit (" << texName << ") not found in shader" << std::endl;
            }
        }
        else
        {
            std::cerr << "Texture uniform (" << texName << ") not found in shader" << std::endl;
        }
    }

    for (const auto& [bufferUnit, bufferName]: desc.buffersMap)
    {
        GLint loc = reflection->getLocation(bufferName);
        if (loc >= 0)
        {
            if (auto& ssboDictionary = reflection->getShaderStorageBufferObjectDictionary();
                ssboDictionary.find(bufferName) != ssboDictionary.end())
            {
                GLint index = getContext().getProgramResourceIndex(
                    shaderStages->getProgram(), GL_SHADER_STORAGE_BLOCK, bufferName.c_str());
                if (index != GL_INVALID_INDEX)
                {
                    bufferUnitMap[bufferUnit] = loc;
                    usingShaderStorageBuffers = true;
                }
                else
                {
                    std::cerr << "Buffer uniform (" << bufferName << ") not found in shader" << std::endl;
                }
            }
            else
            {
                GLint unit = 0;
                getContext().getUniformiv(shaderStages->getProgram(), loc, &unit);
                if (unit >= 0)
                {
                    bufferUnitMap[bufferUnit] = loc;
                }
                else
                {
                    std::cerr << "Buffer uniform unit (" << bufferName << ") not found in shader" << std::endl;
                }
            }
        }
        else
        {
            std::cerr << "Buffer uniform (" << bufferName << ") not found in shader" << std::endl;
        }
    }
}

void ComputePipeline::bind()
{
    if (shaderStages)
    {
        shaderStages->bind();
    }
}

void ComputePipeline::unbind()
{
    if (shaderStages)
    {
        shaderStages->unbind();
    }
}

void ComputePipeline::bindImageUnit(size_t unit, Texture* texture, uint8_t accessFlags, uint32_t mipLevel, uint32_t layer)
{
    if (!shaderStages)
    {
        return;
    }

    if (unit >= MAX_TEXTURE_SAMPLERS)
    {
        return;
    }

    GLint imageUnit = imageUnitMap[unit];
    if (imageUnit >= 0)
    {
        texture->bindImage(imageUnit, accessFlags, mipLevel, layer);
    }
    else
    {
        std::cerr << "Warning: No image found for texture unit: " << unit << std::endl;
    }
}

GLint ComputePipeline::getTextureUnitLocation(size_t unit)
{
    if (unit >= MAX_TEXTURE_SAMPLERS)
    {
        return -1;
    }

    return textureUnitMap[unit];
}

void ComputePipeline::bindBuffer(size_t unit, Buffer* buffer)
{
    if (!shaderStages)
    {
        return;
    }

    if (unit >= MAX_VERTEX_BUFFERS)
    {
        return;
    }

    GLint bufferLocation = bufferUnitMap[unit];
    if (bufferLocation >= 0)
    {
        static_cast<ArrayBuffer&>(*buffer).bindBase(bufferLocation);
    }
    else
    {
        std::cerr << "Warning: No buffer found for unit: " << unit << std::endl;
    }
}

}// namespace opengl
