//
// Created by Jonathan Richard on 2024-03-28.
//

#include "ComputeCommandBuffer.h"
#include "ComputePipeline.h"

namespace opengl {

ComputeCommandBuffer::ComputeCommandBuffer(const std::shared_ptr<Context>& context_)
    : context(context_)
{
}

void ComputeCommandBuffer::begin()
{
    if (isRecording)
    {
        std::cerr << "Command buffer is already recording" << std::endl;
        return;
    }

    isRecording = true;
}

void ComputeCommandBuffer::end()
{
    if (!isRecording)
    {
        std::cerr << "Command buffer is not recording" << std::endl;
        return;
    }

    activeComputePipeline = nullptr;
    imagesCache = {};
    textureStates = {};
    buffersCache = {};

    dirtyBufferUnits.reset();
    dirtyImageUnits.reset();
    dirtyTextureUnits.reset();

    dirtyFlags = DirtyFlag::DirtyBits_None;

    uniformBinder.clearDirtyBufferCache();

    isRecording = false;
}

void ComputeCommandBuffer::bindComputePipeline(const std::shared_ptr<IComputePipeline>& pipeline)
{
    if (activeComputePipeline == pipeline)
    {
        return;
    }

    activeComputePipeline = pipeline;
    setDirty(DirtyFlag::DirtyBits_ComputePipeline);
}

void ComputeCommandBuffer::dispatch(const ThreadGroupDimensions& dimensions)
{
    // prepare compute pipeline
    auto computePipeline = static_pointer_cast<ComputePipeline>(activeComputePipeline);

    if (!computePipeline)
    {
        std::cerr << "No compute pipeline bound" << std::endl;
        return;
    }

    for (size_t i = 0; i < MAX_VERTEX_BUFFERS; ++i)
    {
        if (dirtyBufferUnits.test(i))
        {
            auto& bufferState = buffersCache[i];
            computePipeline->bindBuffer(i, bufferState.buffer.get());
            dirtyBufferUnits.reset(i);
        }
    }

    if (isDirty(DirtyFlag::DirtyBits_ComputePipeline))
    {
        computePipeline->bind();
        clearDirty(DirtyFlag::DirtyBits_ComputePipeline);
    }

    uniformBinder.bindBuffers(*context);

    for (size_t i = 0; i < MAX_TEXTURE_SAMPLERS; ++i)
    {
        if (dirtyImageUnits.test(i))
        {
            auto& image = imagesCache[i];
            computePipeline->bindImageUnit(i, image.texture.get(), image.access, image.mipLevel, image.layer);
            dirtyImageUnits.reset(i);
        }
    }

    for (size_t i = 0; i < MAX_TEXTURE_SAMPLERS; ++i)
    {
        if (dirtyTextureUnits.test(i))
        {
            auto& texture = textureStates[i];

            if (texture.texture)
            {
                GLint loc = computePipeline->getTextureUnitLocation(i);
                if (loc >= 0)
                {
                    context->uniform1i(loc, static_cast<GLint>(i));
                    context->activeTexture(GL_TEXTURE0 + i);

                    texture.texture->bind();
                    if (auto& sampler = texture.samplerState)
                    {
                        sampler->bind(texture.texture);
                    }
                }
                else
                {
                    std::cerr << "Warning: No texture unit found for texture unit: " << i << std::endl;
                }
            }
            dirtyTextureUnits.reset(i);
        }
    }

    // dispatch compute
    context->dispatchCompute(dimensions.x, dimensions.y, dimensions.z);

    // memory barrier
    context->memoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    if (computePipeline->isUsingShaderStorageBuffers())
    {
        context->memoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_ELEMENT_ARRAY_BARRIER_BIT |
                               GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
    }
}

void ComputeCommandBuffer::bindBuffer(size_t index, std::shared_ptr<IBuffer> buffer, size_t offset)
{
    if (index >= MAX_VERTEX_BUFFERS)
    {
        std::cerr << "Buffer index out of range" << std::endl;
        return;
    }

    buffersCache[index].buffer = static_pointer_cast<Buffer>(buffer);
    buffersCache[index].offset = offset;
    dirtyBufferUnits.set(index);
}

void ComputeCommandBuffer::bindImage(size_t index, std::shared_ptr<ITexture> texture, uint8_t accessFlags, uint32_t mipLevel, uint32_t layer)
{
    if (index >= MAX_TEXTURE_SAMPLERS)
    {
        std::cerr << "Texture index out of range" << std::endl;
        return;
    }

//    if (imagesCache[index] == texture)
//    {
//        return;
//    }

    auto& image = imagesCache[index];
    image.texture = static_pointer_cast<Texture>(texture);
    image.mipLevel = mipLevel;
    image.layer = layer;
    image.access = accessFlags;
    dirtyImageUnits.set(index);
}

void ComputeCommandBuffer::bindTexture(size_t index, std::shared_ptr<ITexture> texture)
{
    if (index >= MAX_TEXTURE_SAMPLERS)
    {
        std::cerr << "Texture index out of range" << std::endl;
        return;
    }

//    if (textureStates[index].texture == texture)
//    {
//        return;
//    }

    textureStates[index].texture = static_pointer_cast<Texture>(texture);
    dirtyTextureUnits.set(index);
}

void ComputeCommandBuffer::bindSamplerState(size_t index, std::shared_ptr<ISamplerState> samplerState)
{
    if (index >= MAX_TEXTURE_SAMPLERS)
    {
        std::cerr << "Sampler index out of range" << std::endl;
        return;
    }

    auto& texture = textureStates[index];

    if (!texture.texture)
    {
        std::cerr << "No texture bound to sampler" << std::endl;
        return;
    }

    texture.samplerState = static_pointer_cast<SamplerState>(samplerState);

    dirtyTextureUnits.set(index);
}

bool ComputeCommandBuffer::isDirty(DirtyFlag flag) const
{
    return dirtyFlags & flag;
}
void ComputeCommandBuffer::setDirty(DirtyFlag flag)
{
    dirtyFlags |= flag;
}
void ComputeCommandBuffer::clearDirty(DirtyFlag flag)
{
    dirtyFlags &= ~flag;
}


}