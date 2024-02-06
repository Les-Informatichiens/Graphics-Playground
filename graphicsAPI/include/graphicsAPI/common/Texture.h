//
// Created by Jonathan Richard on 2024-02-02.
//

#pragma once

#include <cstdint>

enum class TextureFormat {
    Undefined        /* determine automatically from supplied data (e.g. image file), if possible */,
    RGB8             /* linear RGB, 8 bits per component */,
    RGBA8            /* linear RGBA, 8 bits per component */,
    SRGB8            /* non-linear sRGB, 8 bits per component */,
    SRGBA8           /* non-linear sRGBA, 8 bits per component (alpha channel linear) */,
    RG16F            /* linear RG, 16-bit floating point components */,
    RGB16F           /* linear RGB, 16-bit floating point components */,
    RGBA16F          /* linear RGBA, 16-bit floating point components */,
    RG32F            /* linear RG, 32-bit floating point components */,
    RGB32F           /* linear RGB, 32-bit floating point components */,
    RGBA32F          /* linear RGBA, 32-bit floating point components */,
    BGR8             /* BGR in sRGB color space, 8 bits per component (except: on Vulkan this one is B10 G11 R11, for 32-bit texels)*/,
    BGRA8            /* BGRA in sRGB color space. 8 bits per component */,
    R8               /* linear single channel, 8 bits */,
    R32F             /* linear single channel, 32-bit floating point */,
    D32F             /* linear depth component, 32-bit floating point */,
    D24S8            /* 24-bit depth component packed with 8-bit stencil component */,
    D32S8            /* 32-bit depth component, 8-bit stencil component */,
    DXT1RGBA         /* Block compressed RGBA */,
    DXT1SRGBA        /* Block compressed SRGBA */,
    DXT3RGBA         /* Block compressed RGBA */,
    DXT3SRGBA        /* Block compressed SRGBA */,
    DXT5RGBA         /* Block compressed RGBA */,
    DXT5SRGBA        /* Block compressed SRGBA */,
    RGTC1R           /* Red Green compressed R */,
    RGTC1SR          /* Red Green compressed signed R */,
    RGTC2RG          /* Red Green compressed RG */,
    RGTC2SRG         /* Red Green compressed signed RG */,
};

// TODO: 1D and 3D textures...
enum class TextureType {
    Undefined,
    Texture2D,
    Texture2DArray,
    TextureCube,
    TextureCubeArray
};


enum class TextureFilter {
    Undefined     /* means guess an appropriate setting from other supplied data */,
    Nearest,
    NearestMipmapNearest,
    NearestMipmapLinear,
    Linear,
    LinearMipmapNearest,
    LinearMipmapLinear
};


enum class TextureWrap {
    Undefined     /* means guess an appropriate setting from other supplied data */,
    ClampToEdge,
    ClampToBorder /* border is always opaque black for now */,
    Repeat,
    MirrorRepeat
};

struct TextureDesc
{
    TextureType type;
    TextureFormat format;

    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t mipLevels;
    uint32_t arrayLayers;
    TextureFilter minFilter;
    TextureFilter magFilter;
    TextureWrap wrapU;
    TextureWrap wrapV;
    TextureWrap wrapW;

    uint32_t usage;
};

struct TextureCopyRegion {
    int32_t srcX = 0;
    int32_t srcY = 0;
    int32_t srcZ = 0;
    uint32_t srcLayer = 0;
    uint32_t srcMipLevel = 0;

    int32_t dstX = 0;
    int32_t dstY = 0;
    int32_t dstZ = 0;
    uint32_t dstLayer = 0;
    uint32_t dstMipLevel = 0;

    uint32_t width = 0;      // 0 means full width
    uint32_t height = 0;     // 0 means full height
    uint32_t depth = 0;      // 0 means full depth
    uint32_t layerCount = 0; // 0 means all layers
    // nb: can only copy 1 mip level at a time
};

class ITexture
{
public:
    virtual ~ITexture() = default;

    // virtual void data(const void* data, size_t size) const = 0;
    virtual void copyFrom(const ITexture& src, TextureCopyRegion region) const = 0;
    virtual void commit() const = 0;

};
