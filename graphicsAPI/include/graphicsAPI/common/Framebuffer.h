//
// Created by Jonathan Richard on 2024-02-02.
//

#pragma once

#include "RenderPass.h"
#include "Texture.h"

#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

struct FramebufferAttachmentDesc
{
    std::shared_ptr<ITexture> texture;
    std::shared_ptr<ITexture> resolveTexture;
};

/**
 * @brief Represents textures associated with the frame buffer including color, depth, and stencil
 * surfaces.
 */
struct FramebufferDesc {
    /** @brief Mapping of index to specific color texture attachments */
    std::unordered_map<size_t, FramebufferAttachmentDesc> colorAttachments;
    /** @brief The depth texture attachment */
    FramebufferAttachmentDesc depthAttachment;
    /** @brief The stencil texture attachment */
    FramebufferAttachmentDesc stencilAttachment;
};

class IFramebuffer
{
public:
    virtual ~IFramebuffer() = default;

    virtual void updateDrawable(std::shared_ptr<ITexture> drawable) = 0;

    [[nodiscard]] virtual std::vector<size_t> getColorAttachmentIndices() const = 0;
    [[nodiscard]] virtual std::shared_ptr<ITexture> getColorAttachment(size_t index) const = 0;
    [[nodiscard]] virtual std::shared_ptr<ITexture> getDepthAttachment() const = 0;
    [[nodiscard]] virtual std::shared_ptr<ITexture> getStencilAttachment() const = 0;
};