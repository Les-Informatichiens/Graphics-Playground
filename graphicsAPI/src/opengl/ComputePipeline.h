//
// Created by Jonathan Richard on 2024-03-28.
//

#pragma once

#include "GraphicsPipelineReflection.h"
#include "graphicsAPI/common/ComputePipeline.h"
#include "graphicsAPI/opengl/Context.h"

#include <array>

namespace opengl {

class Buffer;
class Texture;

class ComputePipeline : public IComputePipeline, public WithContext
{
public:
    explicit ComputePipeline(Context& context, const ComputePipelineDesc& desc);
    ~ComputePipeline() override;

    void initialize(const ComputePipelineDesc& desc);

    void bind();
    void unbind();

    void bindImageUnit(size_t unit, Texture* texture, uint8_t accessFlags, uint32_t mipLevel = 0, uint32_t layer = 0);
    GLint getTextureUnitLocation(size_t unit);
    void bindBuffer(size_t unit, Buffer* buffer);

    bool isUsingShaderStorageBuffers() const { return usingShaderStorageBuffers; }

private:
    using ComputePipelineReflection = GraphicsPipelineReflection;

    std::shared_ptr<PipelineShaderStages> shaderStages;
    std::shared_ptr<ComputePipelineReflection> reflection;

    std::array<GLint, MAX_VERTEX_BUFFERS> bufferUnitMap;
    std::array<GLint, MAX_TEXTURE_SAMPLERS> imageUnitMap;
    std::array<GLint, MAX_TEXTURE_SAMPLERS> textureUnitMap;

    bool usingShaderStorageBuffers = false;
};

}