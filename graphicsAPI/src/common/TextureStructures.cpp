//
// Created by Jonathan Richard on 2024-02-08.
//

#include "graphicsAPI/common/TextureStructures.h"

#include <algorithm>

size_t std::hash<TextureFormat>::operator()(TextureFormat const& key) const {
    return std::hash<size_t>()(static_cast<size_t>(key));
}


TextureRangeDesc TextureRangeDesc::new1D(size_t x, size_t width, size_t mipLevel) {
    return new3D(x, 0, 0, width, 1, 1, mipLevel);
}

TextureRangeDesc TextureRangeDesc::new1DArray(size_t x,
                                              size_t width,
                                              size_t layer,
                                              size_t numLayers,
                                              size_t mipLevel) {
    return new2DArray(x, 0, width, 1, layer, numLayers, mipLevel);
}

TextureRangeDesc TextureRangeDesc::new2D(size_t x,
                                         size_t y,
                                         size_t width,
                                         size_t height,
                                         size_t mipLevel) {
    return new3D(x, y, 0, width, height, 1, mipLevel);
}

TextureRangeDesc TextureRangeDesc::new2DArray(size_t x,
                                              size_t y,
                                              size_t width,
                                              size_t height,
                                              size_t layer,
                                              size_t numLayers,
                                              size_t mipLevel) {
    auto desc = new3D(x, y, 0, width, height, 1, mipLevel);
    desc.layer = layer;
    desc.numLayers = numLayers;
    return desc;
}

TextureRangeDesc TextureRangeDesc::new3D(size_t x,
                                         size_t y,
                                         size_t z,
                                         size_t width,
                                         size_t height,
                                         size_t depth,
                                         size_t mipLevel) {
    TextureRangeDesc desc;
    desc.x = x;
    desc.y = y;
    desc.z = z;
    desc.width = width;
    desc.height = height;
    desc.depth = depth;
    desc.mipLevel = mipLevel;
    return desc;
}

TextureRangeDesc TextureRangeDesc::atMipLevel(size_t newMipLevel) const noexcept {
    if (newMipLevel == mipLevel) {
        return *this;
    } else if (newMipLevel > mipLevel) {
        const auto delta = newMipLevel - mipLevel;
        TextureRangeDesc newRange;
        newRange.x = x >> delta;
        newRange.y = y >> delta;
        newRange.z = z >> delta;
        newRange.width = std::max(width >> delta, static_cast<size_t>(1));
        newRange.height = std::max(height >> delta, static_cast<size_t>(1));
        newRange.depth = std::max(depth >> delta, static_cast<size_t>(1));

        newRange.layer = layer;
        newRange.numLayers = numLayers;

        newRange.mipLevel = newMipLevel;
        newRange.numMipLevels = 1;

        return newRange;
    }

    return *this;
}

TextureRangeDesc TextureRangeDesc::atLayer(size_t newLayer) const noexcept {
    TextureRangeDesc newRange = *this;
    newRange.layer = newLayer;
    newRange.numLayers = 1;

    return newRange;
}

#define PROPERTIES(fmt, cpp, bpb, bw, bh, bd, mbx, mby, mbz, flgs) \
  case TextureFormat::fmt:                                         \
    return TextureFormatProperties{                                \
        TextureFormat::fmt, cpp, bpb, bw, bh, bd, mbx, mby, mbz, flgs};

#define INVALID(fmt) PROPERTIES(fmt, 1, 1, 1, 1, 1, 1, 1, 1, 0)
#define COLOR(fmt, cpp, bpb, flgs) PROPERTIES(fmt, cpp, bpb, 1, 1, 1, 1, 1, 1, flgs)
#define COMPRESSED(fmt, cpp, bpb, bw, bh, bd, mbx, mby, mbz, flgs) \
  PROPERTIES(fmt, cpp, bpb, bw, bh, bd, mbx, mby, mbz, flgs | Flags::Compressed)
#define DEPTH(fmt, cpp, bpb) PROPERTIES(fmt, cpp, bpb, 1, 1, 1, 1, 1, 1, Flags::Depth)
#define DEPTH_STENCIL(fmt, cpp, bpb) \
  PROPERTIES(fmt, cpp, bpb, 1, 1, 1, 1, 1, 1, Flags::Depth | Flags::Stencil)
#define STENCIL(fmt, cpp, bpb) PROPERTIES(fmt, cpp, bpb, 1, 1, 1, 1, 1, 1, Flags::Stencil)

TextureFormatProperties TextureFormatProperties::fromTextureFormat(TextureFormat format) {
    switch (format) {
        INVALID(Invalid)
        COLOR(A_UNorm8, 1, 1, 0)
        COLOR(L_UNorm8, 1, 1, 0)
        COLOR(R_UNorm8, 1, 1, 0)
        COLOR(R_F16, 1, 2, 0)
        COLOR(R_UInt16, 1, 2, 0)
        COLOR(R_UNorm16, 1, 2, 0)
        COLOR(B5G5R5A1_UNorm, 4, 2, 0)
        COLOR(B5G6R5_UNorm, 3, 2, 0)
        COLOR(ABGR_UNorm4, 4, 2, 0)
        COLOR(LA_UNorm8, 2, 2, 0)
        COLOR(RG_UNorm8, 2, 2, 0)
        COLOR(R4G2B2_UNorm_Apple, 3, 2, 0)
        COLOR(R4G2B2_UNorm_Rev_Apple, 3, 2, 0)
        COLOR(R5G5B5A1_UNorm, 4, 2, 0)
        COLOR(RGBX_UNorm8, 3, 3, 0)
        COLOR(RGBA_UNorm8, 4, 4, 0)
        COLOR(BGRA_UNorm8, 4, 4, 0)
        COLOR(BGRA_UNorm8_Rev, 4, 4, 0)
        COLOR(RGBA_SRGB, 4, 4, Flags::sRGB)
        COLOR(BGRA_SRGB, 4, 4, Flags::sRGB)
        COLOR(RG_F16, 2, 4, 0)
        COLOR(RG_UInt16, 2, 4, 0)
        COLOR(RG_UNorm16, 2, 4, 0)
        COLOR(RGB10_A2_UNorm_Rev, 4, 4, 0)
        COLOR(RGB10_A2_Uint_Rev, 4, 4, 0)
        COLOR(BGR10_A2_Unorm, 4, 4, 0)
        COLOR(R_F32, 1, 4, 0)
        COLOR(RGB_F16, 3, 6, 0)
        COLOR(RGBA_F16, 4, 8, 0)
        COLOR(RGB_F32, 3, 12, 0)
        COLOR(RGBA_UInt32, 4, 16, 0)
        COLOR(RGBA_F32, 4, 16, 0)
        COMPRESSED(RGBA_ASTC_4x4, 4, 16, 4, 4, 1, 1, 1, 1, 0)
        COMPRESSED(SRGB8_A8_ASTC_4x4, 4, 16, 4, 4, 1, 1, 1, 1, Flags::sRGB)
        COMPRESSED(RGBA_ASTC_5x4, 4, 16, 5, 4, 1, 1, 1, 1, 0)
        COMPRESSED(SRGB8_A8_ASTC_5x4, 4, 16, 5, 4, 1, 1, 1, 1, Flags::sRGB)
        COMPRESSED(RGBA_ASTC_5x5, 4, 16, 5, 5, 1, 1, 1, 1, 0)
        COMPRESSED(SRGB8_A8_ASTC_5x5, 4, 16, 5, 5, 1, 1, 1, 1, Flags::sRGB)
        COMPRESSED(RGBA_ASTC_6x5, 4, 16, 6, 5, 1, 1, 1, 1, 0)
        COMPRESSED(SRGB8_A8_ASTC_6x5, 4, 16, 6, 5, 1, 1, 1, 1, Flags::sRGB)
        COMPRESSED(RGBA_ASTC_6x6, 4, 16, 6, 6, 1, 1, 1, 1, 0)
        COMPRESSED(SRGB8_A8_ASTC_6x6, 4, 16, 6, 6, 1, 1, 1, 1, Flags::sRGB)
        COMPRESSED(RGBA_ASTC_8x5, 4, 16, 8, 5, 1, 1, 1, 1, 0)
        COMPRESSED(SRGB8_A8_ASTC_8x5, 4, 16, 8, 5, 1, 1, 1, 1, Flags::sRGB)
        COMPRESSED(RGBA_ASTC_8x6, 4, 16, 8, 6, 1, 1, 1, 1, 0)
        COMPRESSED(SRGB8_A8_ASTC_8x6, 4, 16, 8, 6, 1, 1, 1, 1, Flags::sRGB)
        COMPRESSED(RGBA_ASTC_8x8, 4, 16, 8, 8, 1, 1, 1, 1, 0)
        COMPRESSED(SRGB8_A8_ASTC_8x8, 4, 16, 8, 8, 1, 1, 1, 1, Flags::sRGB)
        COMPRESSED(RGBA_ASTC_10x5, 4, 16, 10, 5, 1, 1, 1, 1, 0)
        COMPRESSED(SRGB8_A8_ASTC_10x5, 4, 16, 10, 5, 1, 1, 1, 1, Flags::sRGB)
        COMPRESSED(RGBA_ASTC_10x6, 4, 16, 10, 6, 1, 1, 1, 1, 0)
        COMPRESSED(SRGB8_A8_ASTC_10x6, 4, 16, 10, 6, 1, 1, 1, 1, Flags::sRGB)
        COMPRESSED(RGBA_ASTC_10x8, 4, 16, 10, 8, 1, 1, 1, 1, 0)
        COMPRESSED(SRGB8_A8_ASTC_10x8, 4, 16, 10, 8, 1, 1, 1, 1, Flags::sRGB)
        COMPRESSED(RGBA_ASTC_10x10, 4, 16, 10, 10, 1, 1, 1, 1, 0)
        COMPRESSED(SRGB8_A8_ASTC_10x10, 4, 16, 10, 10, 1, 1, 1, 1, Flags::sRGB)
        COMPRESSED(RGBA_ASTC_12x10, 4, 16, 12, 10, 1, 1, 1, 1, 0)
        COMPRESSED(SRGB8_A8_ASTC_12x10, 4, 16, 12, 10, 1, 1, 1, 1, Flags::sRGB)
        COMPRESSED(RGBA_ASTC_12x12, 4, 16, 12, 12, 1, 1, 1, 1, 0)
        COMPRESSED(SRGB8_A8_ASTC_12x12, 4, 16, 12, 12, 1, 1, 1, 1, Flags::sRGB)
        COMPRESSED(RGBA_PVRTC_2BPPV1, 4, 8, 8, 4, 1, 2, 2, 1, 0)
        COMPRESSED(RGB_PVRTC_2BPPV1, 3, 8, 8, 4, 1, 2, 2, 1, 0)
        COMPRESSED(RGBA_PVRTC_4BPPV1, 4, 8, 4, 4, 1, 2, 2, 1, 0)
        COMPRESSED(RGB_PVRTC_4BPPV1, 3, 8, 4, 4, 1, 2, 2, 1, 0)
        COMPRESSED(RGB8_ETC1, 3, 8, 4, 4, 1, 1, 1, 1, 0)
        COMPRESSED(RGB8_ETC2, 3, 8, 4, 4, 1, 1, 1, 1, 0)
        COMPRESSED(SRGB8_ETC2, 3, 8, 4, 4, 1, 1, 1, 1, Flags::sRGB)
        COMPRESSED(RGB8_Punchthrough_A1_ETC2, 3, 8, 4, 4, 1, 1, 1, 1, 0)
        COMPRESSED(SRGB8_Punchthrough_A1_ETC2, 3, 8, 4, 4, 1, 1, 1, 1, Flags::sRGB)
        COMPRESSED(RGBA8_EAC_ETC2, 4, 16, 4, 4, 1, 1, 1, 1, 0)
        COMPRESSED(SRGB8_A8_EAC_ETC2, 4, 16, 4, 4, 1, 1, 1, 1, Flags::sRGB)
        COMPRESSED(RG_EAC_UNorm, 2, 16, 4, 4, 1, 1, 1, 1, 0)
        COMPRESSED(RG_EAC_SNorm, 2, 16, 4, 4, 1, 1, 1, 1, 0)
        COMPRESSED(R_EAC_UNorm, 1, 8, 4, 4, 1, 1, 1, 1, 0)
        COMPRESSED(R_EAC_SNorm, 1, 8, 4, 4, 1, 1, 1, 1, 0)
        COMPRESSED(RGBA_BC7_UNORM_4x4, 4, 16, 4, 4, 1, 1, 1, 1, 0)
        DEPTH_STENCIL(Z_UNorm16, 1, 2)
        DEPTH_STENCIL(Z_UNorm24, 1, 3)
        DEPTH_STENCIL(Z_UNorm32, 1, 4)
        DEPTH(S8_UInt_Z24_UNorm, 2, 4)
#if IGL_PLATFORM_IOS
        DEPTH(S8_UInt_Z32_UNorm, 2, 5)
#else
        DEPTH(S8_UInt_Z32_UNorm, 2, 8)
#endif
        DEPTH(S_UInt8, 1, 1)
    }
    return {};
}

size_t TextureFormatProperties::getRows(TextureRangeDesc range) const noexcept {
    const auto texHeight = std::max(range.height, static_cast<size_t>(1));
    if (isCompressed()) {
        const size_t heightInBlocks =
                std::max((texHeight + blockHeight - 1) / blockHeight, static_cast<size_t>(minBlocksY));
        return heightInBlocks;
    } else {
        return texHeight;
    }
}

size_t TextureFormatProperties::getBytesPerRow(size_t texWidth) const noexcept {
    return getBytesPerRow(TextureRangeDesc::new1D(0, texWidth));
}

size_t TextureFormatProperties::getBytesPerRow(TextureRangeDesc range) const noexcept {
    const auto texWidth = std::max(range.width, static_cast<size_t>(1));
    if (isCompressed()) {
        const size_t widthInBlocks =
                std::max((texWidth + blockWidth - 1) / blockWidth, static_cast<size_t>(minBlocksX));
        return widthInBlocks * bytesPerBlock;
    } else {
        return texWidth * bytesPerBlock;
    }
}

size_t TextureFormatProperties::getBytesPerLayer(size_t texWidth,
                                                 size_t texHeight,
                                                 size_t texDepth) const noexcept {
    return getBytesPerLayer(TextureRangeDesc::new3D(0, 0, 0, texWidth, texHeight, texDepth));
}

size_t TextureFormatProperties::getBytesPerLayer(TextureRangeDesc range) const noexcept {
    const auto texWidth = std::max(range.width, static_cast<size_t>(1));
    const auto texHeight = std::max(range.height, static_cast<size_t>(1));
    const auto texDepth = std::max(range.depth, static_cast<size_t>(1));
    if (isCompressed()) {
        const size_t widthInBlocks =
                std::max((texWidth + blockWidth - 1) / blockWidth, static_cast<size_t>(minBlocksX));
        const size_t heightInBlocks =
                std::max((texHeight + blockHeight - 1) / blockHeight, static_cast<size_t>(minBlocksY));
        const size_t depthInBlocks =
                std::max((texDepth + blockDepth - 1) / blockDepth, static_cast<size_t>(minBlocksZ));
        return widthInBlocks * heightInBlocks * depthInBlocks * bytesPerBlock;
    } else {
        return texWidth * texHeight * texDepth * bytesPerBlock;
    }
}

size_t TextureFormatProperties::getBytesPerRange(TextureRangeDesc range) const noexcept {
//    IGL_ASSERT(range.x % blockWidth == 0);
//    IGL_ASSERT(range.y % blockHeight == 0);
//    IGL_ASSERT(range.z % blockDepth == 0);
//
    size_t bytes = 0;
    for (size_t i = 0; i < range.numMipLevels; ++i) {
        bytes += getBytesPerLayer(range.atMipLevel(range.mipLevel + i)) * range.numLayers;
    }

    return bytes;
}