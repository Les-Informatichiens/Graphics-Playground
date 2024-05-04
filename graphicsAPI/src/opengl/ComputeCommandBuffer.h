//
// Created by Jonathan Richard on 2024-03-28.
//

#pragma once

#include "graphicsAPI/common/ComputeCommandBuffer.h"
#include "graphicsAPI/opengl/Context.h"

#include "Texture.h"
#include "SamplerState.h"
#include "graphicsAPI/opengl/Buffer.h"
#include "UniformBinder.h"

#include <array>
#include <bitset>

namespace opengl {

class ComputeCommandBuffer : public IComputeCommandBuffer
{
    enum DirtyFlag : uint32_t
    {
        DirtyBits_None = 0,
        DirtyBits_ComputePipeline = 1 << 1,
    };

    struct TextureState
    {
        std::shared_ptr<Texture> texture = nullptr;
        std::shared_ptr<SamplerState> samplerState = nullptr;
    };
    using TextureStates = std::array<TextureState, MAX_TEXTURE_SAMPLERS>;

    struct ImageState
    {
        std::shared_ptr<Texture> texture = nullptr;
        uint32_t mipLevel = 0;
        uint32_t layer = 0;
        uint8_t access = ReadWrite;
    };

    using ImageStates = std::array<ImageState, MAX_TEXTURE_SAMPLERS>;

    struct BufferState
    {
        std::shared_ptr<Buffer> buffer = nullptr;
        size_t offset = 0;
    };
    using BufferStates = std::array<BufferState, MAX_VERTEX_BUFFERS>;

public:
    explicit ComputeCommandBuffer(const std::shared_ptr<Context>& context);
    ~ComputeCommandBuffer() override = default;

    void begin() override;
    void end() override;

    void bindComputePipeline(const std::shared_ptr<IComputePipeline>& pipeline) override;

    void dispatch(const ThreadGroupDimensions& dimensions) override;

    void bindBuffer(size_t index, std::shared_ptr<IBuffer> buffer, size_t offset) override;
    void bindImage(size_t index, std::shared_ptr<ITexture> texture, uint8_t accessFlags, uint32_t mipLevel, uint32_t layer) override;
    void bindTexture(size_t index, std::shared_ptr<ITexture> texture) override;
    void bindSamplerState(size_t index, std::shared_ptr<ISamplerState> samplerState) override;

private:

    [[nodiscard]] bool isDirty(DirtyFlag flag) const;
    void setDirty(DirtyFlag flag);
    void clearDirty(DirtyFlag flag);

private:
    std::bitset<MAX_VERTEX_BUFFERS> dirtyBufferUnits;
    std::bitset<MAX_TEXTURE_SAMPLERS> dirtyImageUnits;
    std::bitset<MAX_TEXTURE_SAMPLERS> dirtyTextureUnits;

    ImageStates imagesCache;
    TextureStates textureStates;
    BufferStates buffersCache;

    std::shared_ptr<IComputePipeline> activeComputePipeline;

    UniformBinder uniformBinder;

    uint32_t dirtyFlags = DirtyFlag::DirtyBits_None;

    std::shared_ptr<Context> context;

    bool isRecording = false;
};

}
