/*
* Copyright (c) Meta Platforms, Inc. and affiliates.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#pragma once

#include <algorithm>
#include <cstdint>

/**
*
* The IGL format name specification is as follows:
*
*  There shall be 3 naming format base types: those for component array
*  formats (type A); those for compressed formats (type C); and those for
*  packed component formats (type P). With type A formats, color component
*  order does not change with endianness. Each format name shall begin with
*  TextureFormat::, followed by a component label (from the Component Label
*  list below) for each component in the order that the component(s) occur
*  in the format, except for non-linear color formats where the first
*  letter shall be 'S'. For type P formats, each component label is
*  followed by the number of bits that represent it in the fundamental
*  data type used by the format.
*
*  Following the listing of the component labels shall be an underscore; a
*  compression type followed by an underscore for Type C formats only; a
*  storage type from the list below; and a bit width for type A formats,
*  which is the bit width for each array element.
*
*  If a format is vendor-specific, then a "_vendor" post fix may be
*  added to the type
*
*
*  ----------    Format Base Type A: Array ----------
*  TextureFormat::[component list]_[storage type][array element bit width][_vendor]
*
*  Examples:
*  TextureFormat::A_SNorm8 - uchar[i] = A
*  TextureFormat::RGBA_SNorm16 - ushort[i * 4 + 0] = R, ushort[i * 4 + 1] = G,
*                                ushort[i * 4 + 2] = B, ushort[i * 4 + 3] = A
*  TextureFormat::Z_UNorm32 - int32[i] = Z
*
*
*  ----------    Format Base Type C: Compressed ----------
*  TextureFormat::[component list#][_*][compression type][_*][block size][_*][storage type#]
*    # where required
*
*  Examples:
*  TextureFormat::RGB_ETC1
*  TextureFormat::RGBA_ASTC_4x4
*  TextureFormat::RGB_PVRTC_2BPPV1
*
*
*  ----------    Format Base Type P: Packed  ----------
*  TextureFormat::[[component list,bit width][storage type#][_]][_][storage type##][_storage
* order###][_vendor#]
*    # when type differs between component
*    ## when type applies to all components
*    ### when storage order is hardware independent
*
*  Examples:
*  TextureFormat::A8B8G8R8_UNorm
*  TextureFormat::R5G6B5_UNorm
*  TextureFormat::B4G4R4X4_UNorm
*  TextureFormat::Z32_F_S8X24_UInt
*  TextureFormat::R10G10B10A2_UInt
*  TextureFormat::R9G9B9E5_F
*  TextureFormat::BGRA_UNorm8_Rev
*
*
*  ----------    Component Labels: ----------
*  A - Alpha
*  B - Blue
*  G - Green
*  I - Intensity
*  L - Luminance
*  R - Red
*  S - Stencil (when not followed by RGB or RGBA)
*  S - non-linear types (when followed by RGB or RGBA)
*  X - Packing bits
*  Z - Depth
*
*  ----------    Storage Types: ----------
*  F: float
*  SInt: Signed Integer
*  UInt: Unsigned Integer
*  SNorm: Signed Normalized Integer/Byte
*  UNorm: Unsigned Normalized Integer/Byte
*
*  ----------    Type C Compression Types: Additional Info ----------
*  ETC1 - No other information required
*  ETC2 - No other information required
*  ASTC - Block size shall be given
*  PVRTC - Block size shall be given
*
*/

enum class TextureFormat : uint8_t {
   Invalid = 0,

   // 8 bpp
   A_UNorm8,
   L_UNorm8,
   R_UNorm8,

   // 16 bpp
   R_F16,
   R_UInt16,
   R_UNorm16,
   B5G5R5A1_UNorm,
   B5G6R5_UNorm,
   ABGR_UNorm4, // NA on GLES
   LA_UNorm8,
   RG_UNorm8,
   R4G2B2_UNorm_Apple,
   R4G2B2_UNorm_Rev_Apple,
   R5G5B5A1_UNorm,

   // 24 bpp
   RGBX_UNorm8,

   // 32 bpp
   RGBA_UNorm8,
   BGRA_UNorm8,
   BGRA_UNorm8_Rev,
   RGBA_SRGB,
   BGRA_SRGB,
   RG_F16,
   RG_UInt16,
   RG_UNorm16,
   RGB10_A2_UNorm_Rev,
   RGB10_A2_Uint_Rev,
   BGR10_A2_Unorm,
   R_F32,
   // 48 bpp
   RGB_F16,

   // 64 bpp
   RGBA_F16,

   // 96 bpp
   RGB_F32,

   // 128 bpp
   RGBA_UInt32,
   RGBA_F32,

   // Compressed
   RGBA_ASTC_4x4,
   SRGB8_A8_ASTC_4x4,
   RGBA_ASTC_5x4,
   SRGB8_A8_ASTC_5x4,
   RGBA_ASTC_5x5,
   SRGB8_A8_ASTC_5x5,
   RGBA_ASTC_6x5,
   SRGB8_A8_ASTC_6x5,
   RGBA_ASTC_6x6,
   SRGB8_A8_ASTC_6x6,
   RGBA_ASTC_8x5,
   SRGB8_A8_ASTC_8x5,
   RGBA_ASTC_8x6,
   SRGB8_A8_ASTC_8x6,
   RGBA_ASTC_8x8,
   SRGB8_A8_ASTC_8x8,
   RGBA_ASTC_10x5,
   SRGB8_A8_ASTC_10x5,
   RGBA_ASTC_10x6,
   SRGB8_A8_ASTC_10x6,
   RGBA_ASTC_10x8,
   SRGB8_A8_ASTC_10x8,
   RGBA_ASTC_10x10,
   SRGB8_A8_ASTC_10x10,
   RGBA_ASTC_12x10,
   SRGB8_A8_ASTC_12x10,
   RGBA_ASTC_12x12,
   SRGB8_A8_ASTC_12x12,
   RGBA_PVRTC_2BPPV1,
   RGB_PVRTC_2BPPV1,
   RGBA_PVRTC_4BPPV1,
   RGB_PVRTC_4BPPV1,
   RGB8_ETC1,
   RGB8_ETC2,
   SRGB8_ETC2,
   RGB8_Punchthrough_A1_ETC2,
   SRGB8_Punchthrough_A1_ETC2,
   RGBA8_EAC_ETC2,
   SRGB8_A8_EAC_ETC2,
   RG_EAC_UNorm,
   RG_EAC_SNorm,
   R_EAC_UNorm,
   R_EAC_SNorm,
   RGBA_BC7_UNORM_4x4, // block compression

   // Depth and Stencil formats
   Z_UNorm16, // NA on iOS/Metal but works on iOS GLES. The client has to account for
             // this!
   Z_UNorm24,
   Z_UNorm32, // NA on iOS/GLES but works on iOS Metal. The client has to account for
                     // this!
   S8_UInt_Z24_UNorm, // NA on iOS
   S8_UInt_Z32_UNorm, // NA on iOS/GLES but works on iOS Metal. The client has to
                     // account for this!
   S_UInt8
};

constexpr bool isTextureFormatsRGB(TextureFormat textureFormat) {
   return textureFormat == TextureFormat::RGBA_SRGB || textureFormat == TextureFormat::BGRA_SRGB;
}

constexpr bool isTextureFormatBGR(TextureFormat textureFormat) {
   return textureFormat == TextureFormat::BGRA_SRGB || textureFormat == TextureFormat::BGRA_UNorm8;
}


/**
 * @brief Descriptor for texture dimensions
 *
 *  x         - offset position in width
 *  y         - offset position in height
 *  z         - offset position in depth
 *  width     - width of the range
 *  height    - height of the range
 *  depth     - depth of the range
 *  layer     - layer offset for 1D/2D array textures. Not used for cube textures faces.
 *  numLayers - number of layers in the range
 *  mipLevel  - mipmap level of the range
 */
struct TextureRangeDesc {
    size_t x = 0;
    size_t y = 0;
    size_t z = 0;
    size_t width = 1;
    size_t height = 1;
    size_t depth = 1;
    size_t layer = 0;
    size_t numLayers = 1;
    size_t mipLevel = 0;
    // number of mip levels to update
    size_t numMipLevels = 1;

    static TextureRangeDesc new1D(size_t x, size_t width, size_t mipLevel = 0);
    static TextureRangeDesc new1DArray(size_t x,
                                       size_t width,
                                       size_t layer,
                                       size_t numLayers,
                                       size_t mipLevel = 0);
    static TextureRangeDesc new2D(size_t x,
                                  size_t y,
                                  size_t width,
                                  size_t height,
                                  size_t mipLevel = 0);
    static TextureRangeDesc new2DArray(size_t x,
                                       size_t y,
                                       size_t width,
                                       size_t height,
                                       size_t layer,
                                       size_t numLayers,
                                       size_t mipLevel = 0);
    static TextureRangeDesc new3D(size_t x,
                                  size_t y,
                                  size_t z,
                                  size_t width,
                                  size_t height,
                                  size_t depth,
                                  size_t mipLevel = 0);

    /**
   * @brief Returns a new TextureRangeDesc based on this one but reduced to the specified mipLevel.
   *
   * @param newMipLevel The mip level of the returned range.
   * @remark The returned range only has 1 mip level.
   */
    [[nodiscard]] TextureRangeDesc atMipLevel(size_t newMipLevel) const noexcept;
    /**
   * @brief Returns a new TextureRangeDesc based on this one but reduced to the specified layer.
   *
   * @param newLayer The layer of the returned range.
   * @remark The returned range only has 1 layer.
   */
    [[nodiscard]] TextureRangeDesc atLayer(size_t newLayer) const noexcept;
};

/**
 * @brief Encapsulates properties of a texture format
 *
 *  name               - Stringified IGL enum for the format
 *  format             - IGL enum for the format
 *  componentsPerPixel - Number of components for each pixel (e.g., RGB has 3)
 *  bytesPerBlock      - Bytes per pixel block (compressed) or per pixel (uncompressed)
 *  blockWidth         - Block width for compressed textures (always 1 for uncompressed)
 *  blockHeight        - Block height for compressed textures (always 1 for uncompressed)
 *  blockDepth         - Block depth for compressed textures (always 1 for uncompressed)
 *  minBlocksX         - Minimum number of blocks in the X direction for compressed textures
 *  minBlocksY         - Minimum number of blocks in the Y direction for compressed textures
 *  minBlocksZ         - Minimum number of blocks in the Z direction for compressed textures
 *  flags              - Additional boolean flags for the format:
 *                        - Depth:      Depth texture format
 *                        - Stencil:    Stencil texture format
 *                        - Compressed: Compressed texture format
 *                        - sRGB:       sRGB texture format
 */
struct TextureFormatProperties {
    static TextureFormatProperties fromTextureFormat(TextureFormat format);

    enum Flags : uint8_t {
        Depth = 1 << 0,
        Stencil = 1 << 1,
        Compressed = 1 << 2,
        sRGB = 1 << 3,
    };

    const TextureFormat format = TextureFormat::Invalid;
    const uint8_t componentsPerPixel = 1;
    const uint8_t bytesPerBlock = 1;
    const uint8_t blockWidth = 1;
    const uint8_t blockHeight = 1;
    const uint8_t blockDepth = 1;
    const uint8_t minBlocksX = 1;
    const uint8_t minBlocksY = 1;
    const uint8_t minBlocksZ = 1;
    const uint8_t flags = 0;

    /**
   * @brief true for TextureFormat::Invalid
   */
    [[nodiscard]] bool isValid() const noexcept {
        return format != TextureFormat::Invalid;
    }
    /**
   * @brief true compressed texture formats.
   */
    [[nodiscard]] bool isCompressed() const noexcept {
        return (flags & Flags::Compressed) != 0;
    }
    /**
   * @brief true sRGB texture formats.
   */
    [[nodiscard]] bool isSRGB() const noexcept {
        return (flags & Flags::sRGB) != 0;
    }
    /**
   * @brief true depth-only texture formats (e.g., TextureFormat::Z_UNorm24).
   */
    [[nodiscard]] bool isDepthOnly() const noexcept {
        return (flags & Flags::Depth) != 0 && (flags & Flags::Stencil) == 0;
    }
    /**
   * @brief true stencil-only texture formats (e.g., TextureFormat::S_UInt8).
   */
    [[nodiscard]] bool isStencilOnly() const noexcept {
        return (flags & Flags::Depth) == 0 && (flags & Flags::Stencil) != 0;
    }
    /**
   * @brief true depth-only, stencil-only and depth-stencil texture formats.
   */
    [[nodiscard]] bool isDepthOrStencil() const noexcept {
        return (flags & Flags::Depth) != 0 || (flags & Flags::Stencil) != 0;
    }

    /**
   * @brief Utility function to calculate the number of rows in the range for the texture format.
   * For uncompressed textures, this will be range.height. For compressed textures, range.height
   * rounded up to the nearest multiple of blockHeight.
   *
   * @param range  range.width, range.height, and range.depth should be the actual dimensions of the
   * range to calculate for. For subranges and mip levels other than 0, these should be the
   * dimensions of the subrange and/or mip level, which may be be less than the full texture
   * dimensions.
   * @return Calculated number of rows of texture data for the texture format.
   */
    size_t getRows(TextureRangeDesc range) const noexcept;

    /**
   * @brief Utility function to calculate the size in bytes per row for a texture format.
   *
   * @param texWidth  The width, in pixels, of the texture data. This should be the row width to
   * calculate for. For subranges and mip levels other than 0, this should be the width of the
   * subrange and/or mip level, which may be be less than the full texture width.
   * @return Calculated total size in bytes of a row of texture data for the texture format.
   */
    [[nodiscard]] size_t getBytesPerRow(size_t texWidth) const noexcept;

    /**
   * @brief Utility function to calculate the size in bytes per row for a texture format.
   *
   * @param range  The range, in pixels, of the texture data row. range.width be the row width to
   * calculate for. For subranges and mip levels other than 0, this should be the width of the
   * subrange and/or mip level, which may be be less than the full texture width.
   * @return Calculated total size in bytes of a row of texture data for the texture format.
   */
    [[nodiscard]] size_t getBytesPerRow(TextureRangeDesc range) const noexcept;

    /**
   * @brief Utility function to calculate the size in bytes per texture layer for a texture format.
   *
   * @param texWidth  The width of the texture layer.
   * @param texHeight  The height of the texture layer.
   * @param texDepth  The depth of the texture layer.
   * @remark texWidth, texHeight and texDepth should be the actual dimensions of the range to
   * calculate for. For subranges and mip levels other than 0, this should be the dimensions of the
   * subrange and/or mip level, which may be be less than the full texture dimensions.
   * @return Calculated total size in bytes of a layer of texture data for the texture format.
   */
    [[nodiscard]] size_t getBytesPerLayer(size_t texWidth,
                                          size_t texHeight,
                                          size_t texDepth) const noexcept;

    /**
   * @brief Utility function to calculate the size in bytes per texture layer for a texture format.
   *
   * @param range  The range of the texture to calculate for. The range should be the full size of
   * the first mip level to calculate for. range.x, range.y, range.z, range.mipLevel and range.layer
   * are not used.
   * @remark range.width, range.height, and range.depth should be the actual dimensions of the range
   * to calculate for. For subranges and mip levels other than 0, these should be the dimensions of
   * the subrange and/or mip level, which may be be less than the full texture dimensions.
   * @return Calculated total size in bytes of a layer of texture data for the texture format.
   */
    size_t getBytesPerLayer(TextureRangeDesc range) const noexcept;

    /**
   * @brief Utility function to calculate the size in bytes per texture range for a texture format.
   *
   * @param range  The range of the texture to calculate for. The range should be the full size of
   * the first mip level to calculate for. range.x, range.y and range.z are not used.
   * @remark range can include more than one layer. range can also include more than one mip level.
   * When range includes more than one mip level, dimensions  are divided by two for each subsequent
   * mip level.
   * @return Calculated total size in bytes of a the range of texture data for the texture format.
   */
    [[nodiscard]] size_t getBytesPerRange(TextureRangeDesc range) const noexcept;
};

namespace std {

// Add a hash function for older compilers
template<>
struct hash<TextureFormat> {
    // Declare member
    size_t operator()(TextureFormat const& /*key*/) const;
};

} // namespace std
