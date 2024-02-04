//
// Created by Jonathan Richard on 2024-02-02.
//

#pragma once

#include "Texture.h"

#include <memory>
#include <optional>
#include <vector>

enum class AttachmentType
{
    Color,
    Depth,
    Stencil
};


struct FramebufferAttachmentDesc
{
    AttachmentType type;
    TextureType textureType;
    TextureFormat format;
};

struct FramebufferDesc
{
    uint32_t width;
    uint32_t height;
    uint32_t layers;
    std::vector<FramebufferAttachmentDesc> colorAttachments;
    std::optional<FramebufferAttachmentDesc> depthAttachment;
    std::optional<FramebufferAttachmentDesc> stencilAttachment;
};

class IFramebuffer
{
public:
    virtual ~IFramebuffer() = default;

    [[nodiscard]] virtual std::vector<size_t> getColorAttachmentIndices() const = 0;
    [[nodiscard]] virtual std::shared_ptr<ITexture> getColorAttachment(size_t index) const = 0;
    [[nodiscard]] virtual std::shared_ptr<ITexture> getDepthAttachment() const = 0;
    [[nodiscard]] virtual std::shared_ptr<ITexture> getStencilAttachment() const = 0;
};