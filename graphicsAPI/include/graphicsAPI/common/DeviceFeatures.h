//
// Created by Jonathan Richard on 2024-02-03.
//

#pragma once

#include "Texture.h"
#include <cstdint>

enum class DeviceFeatures {
    BindBytes = 0,
    BindUniform,
    BufferDeviceAddress,
    BufferNoCopy,
    BufferRing,
    Compute,
    DepthCompare,
    DepthShaderRead,
    DrawIndexedIndirect,
    ExplicitBinding,
    ExplicitBindingExt,
    ExternalMemoryObjects,
    MapBufferRange,
    MinMaxBlend,
    MultipleRenderTargets,
    MultiSample,
    MultiSampleResolve,
    Multiview,
    PushConstants,
    ReadWriteFramebuffer,
    SamplerMinMaxLod,
    ShaderLibrary,
    ShaderTextureLod,
    ShaderTextureLodExt,
    SRGB,
    SRGBWriteControl,
    StandardDerivative,
    StandardDerivativeExt,
    StorageBuffers,
    Texture2DArray,
    TextureArrayExt,
    Texture3D,
    TextureBindless,
    TextureExternalImage,
    TextureFilterAnisotropic,
    TextureFloat,
    TextureFormatRG,
    TextureFormatRGB,
    TextureHalfFloat,
    TextureNotPot,
    TexturePartialMipChain,
    UniformBlocks,
    ValidationLayersEnabled,
};

enum class ShaderFamily
{
    Unknown,
    GLSL,
    GLSL_ES,
    SPIRV
};

struct ShaderVersion
{
    ShaderFamily family;
    uint8_t major;
    uint8_t minor;

    bool operator==(const ShaderVersion& other) const {
        return family == other.family && major == other.major &&
               minor == other.minor;
    }

    bool operator!=(const ShaderVersion& other) const {
        return !(*this == other);
    }
};

class ICapabilities {
public:
    /**
   * @brief TextureFormatCapabilityBits provides specific texture format usage
   *
   * Unsupported       The format is not supported
   * Sampled           Can be used as read-only texture in vertex/fragment shaders
   * SampledFiltered   The texture can be filtered during sampling
   * Filter            The texture can be filtered during sampling
   * Storage           Can be used as read/write storage texture in vertex/fragment/compute shaders
   * Attachment        The texture can be used as a render target
   * SampledAttachment The texture can be both sampled in shaders AND used as a render target
   * All               All capabilities are supported
   *
   * @remark SampledAttachment is NOT the same as Sampled | Attachment.
   */
    enum TextureFormatCapabilityBits : uint8_t {
        Unsupported = 0,
        Sampled = 1 << 0,
        SampledFiltered = 1 << 1,
        Storage = 1 << 2,
        Attachment = 1 << 3,
        SampledAttachment = 1 << 4,
        All = Sampled | SampledFiltered | Storage | Attachment | SampledAttachment
    };

    using TextureFormatCapabilities = uint8_t;

    /**
   * @brief This function indicates if a device feature is supported at all.
   *
   * @param feature The device feature specified
   * @return True,  If feature is supported
   *         False, Otherwise
   */
    virtual bool hasFeature(DeviceFeatures feature) const = 0;

    /**
   * @brief This function gets capabilities of a specified texture format
   *
   * @param format The texture format
   * @return TextureFormatCapabilities
   */
    virtual TextureFormatCapabilities getTextureFormatCapabilities(TextureFormat format) const = 0;

    /**
   * @brief Gets the latest shader language version supported by this device.
   * @return ShaderVersion
   */
    virtual ShaderVersion getShaderVersion() const = 0;

protected:
    virtual ~ICapabilities() = default;
};

inline bool contains(ICapabilities::TextureFormatCapabilities value,
                     ICapabilities::TextureFormatCapabilityBits flag) {
    return (value & flag) == flag;
}