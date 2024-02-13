//
// Created by Jonathan Richard on 2024-02-01.
//

#pragma once

#include <vector>

/**
 * @brief LoadAction determines the load action of the various components of a RenderPassDesc. This can be DontCare, Load, or Clear.
 *
 * DontCare : No specific operation required.
 * Load : Load the contents of the attachment
 * Clear : Clear the contents of the attachment
 */
enum class LoadAction : uint8_t {
    DontCare,
    Load,
    Clear,
};

/**
 * @brief StoreAction determines the resolution action of the various components of a
 * RenderPassDesc. This can be DontCare, Store, or MsaaResolve.
 *
 * DontCare : No specific operation required.
 * Store : Preserve render contents
 * MsaaResolve : Use MSAA to preserve render contents
 */
enum class StoreAction : uint8_t {
    DontCare,
    Store,
    MsaaResolve,
};

/**
 * @brief MsaaDepthResolveFilter determines the MSAA algorithm used. This can be Sample0, Min, or
 * Max.
 *
 * Sample0 : Use provided value
 * Min : Use minimum value allowed
 * Max : Use maximum value allowed
 */
enum class MsaaDepthResolveFilter : uint8_t {
    Sample0,
    Min,
    Max,
};

/**
 * @brief This class will be used to represent a render pass in both the Vulkan and OpenGL implementations.
 * It will be used to describe the operations that will be performed on the attachments of a framebuffer during a "render pass".
 */
struct RenderPassDesc
{
    struct BaseAttachmentDesc
    {
        LoadAction loadAction = LoadAction::Clear; // default load action for depth and stencil
        StoreAction storeAction = StoreAction::DontCare; // default store action for depth and stencil
        uint8_t layer = 0;
        uint8_t mipmapLevel = 0;
    };

    /**
   * @brief ColorAttachmentDesc stores to black by default on a RenderPassDesc but can be set to
   * perform other operations
   */
    struct ColorAttachmentDesc : BaseAttachmentDesc {
        // NOTE: Color has different defaults
        ColorAttachmentDesc(LoadAction loadAction, StoreAction storeAction, Color clearColor = {0.0f, 0.0f, 0.0f, 0.0f}, uint8_t layer = 0, uint8_t mipmapLevel = 0)
            : BaseAttachmentDesc{loadAction, storeAction, layer, mipmapLevel}, clearColor{clearColor} {}
        ColorAttachmentDesc() : BaseAttachmentDesc{LoadAction::DontCare, StoreAction::Store} {}
        Color clearColor = {0.0f, 0.0f, 0.0f, 0.0f};
    };

    /**
   * @brief DepthAttachmentDesc uses provided MSAA value by default and clears depth.
   */
    struct DepthAttachmentDesc : BaseAttachmentDesc {
//        MsaaDepthResolveFilter depthResolveFilter = MsaaDepthResolveFilter::Sample0;
        float clearDepth = 1.0f;
    };

    /**
   * @brief StencilAttachmentDesc sets stencil to 0 by default.
   */
    struct StencilAttachmentDesc : BaseAttachmentDesc {
        uint32_t clearStencil = 0;
    };

    /**
   * @brief colorAttachments properties which is empty by default.
   */
    std::vector<ColorAttachmentDesc> colorAttachments;
    /**
   * @brief depthAttachment property which is clear to 1 and use provided MSAA by default
   */
    DepthAttachmentDesc depthAttachment;
    /**
   * @brief stencilAttachment property which is clear to 0 by default
   */
    StencilAttachmentDesc stencilAttachment;
};