//
// Created by Jonathan Richard on 2024-02-07.
//

#include "Texture.h"

namespace opengl {

Texture::Texture(Context& context, TextureFormat format)
    : WithContext(context), formatProperties(TextureFormatProperties::fromTextureFormat(format))
{
}

void Texture::upload(const void* data, const TextureRangeDesc& range, size_t bytesPerRow) const
{
}

void Texture::uploadCube(const void* data, TextureCubeFace face, const TextureRangeDesc& range, size_t bytesPerRow) const
{
}

bool Texture::toFormatDescGL(TextureFormat textureFormat, TextureDesc::TextureUsage usage, FormatDescGL& outFormatGL)
{
    bool sampled = (usage & TextureDesc::TextureUsageBits::Sampled) != 0;
    bool attachment = (usage & TextureDesc::TextureUsageBits::Attachment) != 0;
    bool storage = (usage & TextureDesc::TextureUsageBits::Storage) != 0;
    bool sampledAttachment = sampled && attachment;
    bool sampledOnly = sampled && !attachment;
    bool attachmentOnly = attachment && !sampled;

    const bool renderbuffer = attachmentOnly;
    const bool texStorage = storage;
    const bool texImage = !storage && sampled;
    if (!renderbuffer && !texStorage && !texImage)
    {
        return false;
    }

    const bool compressedTexStorage = storage;
    const bool compressedTexImage = sampled;
    const bool compressedValid = compressedTexStorage || compressedTexImage;

    auto& format = outFormatGL.format;
    auto& type = outFormatGL.type;
    auto& internalFormat = outFormatGL.internalFormat;

    switch (textureFormat)
    {
        case TextureFormat::Invalid:
            return false;

        case TextureFormat::RGBA_UNorm8:
            format = GL_RGBA;
            type = GL_UNSIGNED_BYTE;
            internalFormat = GL_RGBA8;
            if (texImage /*&& !deviceFeatures.hasTextureFeature(TextureFeatures::ColorTexImageRgba8)*/)
            {
                internalFormat = GL_RGBA;
            }
            return true;

        case TextureFormat::RGBA_SRGB:
            format = /*deviceFeatures.hasExtension(Extensions::Srgb) ? GL_SRGB_ALPHA : */ GL_RGBA;
            type = GL_UNSIGNED_BYTE;
            internalFormat = GL_SRGB8_ALPHA8;
            if (texImage /*&& !deviceFeatures.hasTextureFeature(TextureFeatures::ColorTexImageSrgba8)*/)
            {
                internalFormat = GL_SRGB_ALPHA;
            }
            return true;

        case TextureFormat::BGRA_SRGB:
            format = GL_BGRA;
            type = GL_UNSIGNED_BYTE;
            internalFormat = GL_SRGB8_ALPHA8;
            if (texImage /* && !deviceFeatures.hasTextureFeature(TextureFeatures::ColorTexImageSrgba8)*/)
            {
                internalFormat = GL_SRGB_ALPHA;
            }
            return true;

        case TextureFormat::R4G2B2_UNorm_Apple:
            format = GL_RGB_422_APPLE;
            type = GL_UNSIGNED_SHORT_8_8_APPLE;
            internalFormat = GL_RGB_RAW_422_APPLE;
            //            if (texImage && deviceFeatures.hasInternalRequirement(
            //                                    InternalRequirement::ColorTexImageRgbApple422Unsized)) {
            //                internalFormat = GL_RGB;
            //            }
            return true;

        case TextureFormat::R4G2B2_UNorm_Rev_Apple:
            format = GL_RGB_422_APPLE;
            type = GL_UNSIGNED_SHORT_8_8_REV_APPLE;
            internalFormat = GL_RGB_RAW_422_APPLE;
            //            if (texImage && deviceFeatures.hasInternalRequirement(
            //                                    InternalRequirement::ColorTexImageRgbApple422Unsized)) {
            //                internalFormat = GL_RGB;
            //            }
            return true;

        case TextureFormat::R5G5B5A1_UNorm:
            format = GL_RGBA;
            type = GL_UNSIGNED_SHORT_5_5_5_1;
            internalFormat = GL_RGB5_A1;
            //            if (texImage &&
            //                deviceFeatures.hasInternalRequirement(InternalRequirement::ColorTexImageRgb5A1Unsized)) {
            //                internalFormat = GL_RGBA;
            //            }
            return true;

        case TextureFormat::RGBX_UNorm8:
            format = GL_RGB;
            type = GL_UNSIGNED_BYTE;
            internalFormat = GL_RGB8;
            if (texImage /*&& !deviceFeatures.hasTextureFeature(TextureFeatures::ColorTexImageRgba8)*/)
            {
                internalFormat = GL_RGB;
            }
            return true;

        case TextureFormat::RGBA_F32:
            format = GL_RGBA;
            type = GL_FLOAT;
            internalFormat = GL_RGBA32F;
            if (texImage /*&& !deviceFeatures.hasTextureFeature(TextureFeatures::ColorTexImage32f)*/)
            {
                internalFormat = GL_RGBA;
            }
            return true;

        case TextureFormat::RGB_F32:
            format = GL_RGB;
            type = GL_FLOAT;
            internalFormat = GL_RGB32F;
            if (texImage /*&& !deviceFeatures.hasTextureFeature(TextureFeatures::ColorTexImage32f)*/)
            {
                internalFormat = GL_RGB;
            }
            return true;

        case TextureFormat::RGBA_F16:
            format = GL_RGBA;
            //            if (deviceFeatures.hasInternalRequirement(InternalRequirement::TextureHalfFloatExtReq)) {
            //                type = GL_HALF_FLOAT_OES; // NOTE: NOT the same as GL_HALF_FLOAT
            //            } else {
            type = GL_HALF_FLOAT;
            //            }
            internalFormat = GL_RGBA16F;
            if (texImage /* && !deviceFeatures.hasTextureFeature(TextureFeatures::ColorTexImage16f)*/)
            {
                internalFormat = GL_RGBA;
            }
            return true;

        case TextureFormat::RGB_F16:
            format = GL_RGB;
            //            if (deviceFeatures.hasInternalRequirement(InternalRequirement::TextureHalfFloatExtReq)) {
            //                type = GL_HALF_FLOAT_OES; // NOTE: NOT the same as GL_HALF_FLOAT
            //            } else {
            type = GL_HALF_FLOAT;
            //            }
            internalFormat = GL_RGB16F;
            if (texImage /*&& !deviceFeatures.hasTextureFeature(TextureFeatures::ColorTexImage16f)*/)
            {
                internalFormat = GL_RGB;
            }
            return true;

        case TextureFormat::BGRA_UNorm8:
            format = GL_BGRA;
            type = GL_UNSIGNED_BYTE;
            internalFormat = GL_RGBA;
            if (texStorage)
            {
                internalFormat = GL_BGRA8_EXT;
            } /*else if (texImage && deviceFeatures.hasExtension(Extensions::TextureFormatBgra8888Ext)) {
                internalFormat = GL_BGRA;
            } else if (texImage &&
                     deviceFeatures.hasTextureFeature(TextureFeatures::ColorTexImageBgraRgba8)) {
                internalFormat = GL_RGBA8;
            }*/
            return true;

        case TextureFormat::BGRA_UNorm8_Rev:
            internalFormat = GL_RGBA;
            format = GL_BGRA;
            type = GL_UNSIGNED_INT_8_8_8_8_REV;
            return true;

        case TextureFormat::RGB10_A2_UNorm_Rev:
            format = GL_RGBA;
            type = GL_UNSIGNED_INT_2_10_10_10_REV;
            internalFormat = GL_RGB10_A2;
            //            if (texImage &&
            //                deviceFeatures.hasInternalRequirement(InternalRequirement::ColorTexImageRgb10A2Unsized)) {
            //                internalFormat = GL_RGBA;
            //            }
            return true;

        case TextureFormat::RGB10_A2_Uint_Rev:
            internalFormat = GL_RGB10_A2UI;
            format = GL_RGBA_INTEGER;
            type = GL_UNSIGNED_INT_2_10_10_10_REV;
            return true;

        case TextureFormat::BGR10_A2_Unorm:
            format = GL_BGRA;
            type = GL_UNSIGNED_INT_2_10_10_10_REV;
            internalFormat = GL_RGB10_A2;
            return true;

        case TextureFormat::ABGR_UNorm4:// TODO Test this
            format = GL_RGBA;
            type = GL_UNSIGNED_SHORT_4_4_4_4;
            internalFormat = GL_RGBA4;
            //            if (texImage &&
            //                deviceFeatures.hasInternalRequirement(InternalRequirement::ColorTexImageRgba4Unsized)) {
            //                internalFormat = GL_RGBA;
            //            }
            return true;

        case TextureFormat::B5G5R5A1_UNorm:
            format = GL_BGRA;
            type = GL_UNSIGNED_SHORT_5_5_5_1;
            internalFormat = GL_RGB5_A1;
            //            if (texImage &&
            //                deviceFeatures.hasInternalRequirement(InternalRequirement::ColorTexImageRgb5A1Unsized)) {
            //                internalFormat = GL_RGBA;
            //            }
            return true;

        case TextureFormat::B5G6R5_UNorm:
            return false;

        case TextureFormat::LA_UNorm8:
            format = GL_LUMINANCE_ALPHA;
            type = GL_UNSIGNED_BYTE;
            internalFormat = GL_LUMINANCE_ALPHA;
            if ((texStorage /* && deviceFeatures.hasTextureFeature(TextureFeatures::ColorTexStorageLa8)*/) ||
                (texImage /* && deviceFeatures.hasTextureFeature(TextureFeatures::ColorTexImageLa8)*/))
            {
                internalFormat = GL_LUMINANCE8_ALPHA8;
            }
            return true;

        case TextureFormat::RG_UNorm8:
            format = GL_RG;
            type = GL_UNSIGNED_BYTE;
            internalFormat = GL_RG8;
            if (texImage /*&& !deviceFeatures.hasTextureFeature(TextureFeatures::ColorTexImageRg8)*/)
            {
                internalFormat = GL_RG;
            }
            return true;

        case TextureFormat::RG_F16:
            format = GL_RG;
            //            if (deviceFeatures.hasInternalRequirement(InternalRequirement::TextureHalfFloatExtReq)) {
            //                type = GL_HALF_FLOAT_OES; // NOTE: NOT the same as GL_HALF_FLOAT
            //            } else {
            type = GL_HALF_FLOAT;
            //            }
            internalFormat = GL_RG16F;
            if (texImage /*&& !deviceFeatures.hasTextureFeature(TextureFeatures::ColorTexImage16f)*/)
            {
                internalFormat = GL_RG;
            }
            return true;

        case TextureFormat::RG_UInt16:
            internalFormat = GL_RG16UI;
            format = GL_RG_INTEGER;
            type = GL_UNSIGNED_SHORT;
            return true;

        case TextureFormat::RG_UNorm16:
            internalFormat = GL_RG16;
            format = GL_RG;
            type = GL_UNSIGNED_SHORT;
            return true;

        case TextureFormat::RGBA_UInt32:
            internalFormat = GL_RGBA32UI;
            format = GL_RGBA_INTEGER;
            type = GL_UNSIGNED_INT;
            return true;

        case TextureFormat::A_UNorm8:
            type = GL_UNSIGNED_BYTE;
            format = GL_ALPHA;
            internalFormat = GL_ALPHA;
            if ((texImage /* && deviceFeatures.hasTextureFeature(TextureFeatures::ColorTexImageA8)*/) ||
                (texStorage /* && deviceFeatures.hasTextureFeature(TextureFeatures::ColorTexStorageA8)*/))
            {
                //                if (deviceFeatures.hasInternalRequirement(InternalRequirement::SwizzleAlphaTexturesReq)) {
                // GL_ALPHA was deprecated in GL3 so use GL_RED and use GL_TEXTURE_SWIZZLE_A in
                // swapTextureChannelsForFormat before calling texImage2D or texStorage2D
                //                    internalFormat = GL_R8;
                //                    format = GL_RED;
                //                } else {
                internalFormat = GL_ALPHA8;
                //                }
            }
            return true;

        case TextureFormat::L_UNorm8:
            format = GL_LUMINANCE;
            type = GL_UNSIGNED_BYTE;
            internalFormat = GL_LUMINANCE;
            if ((texStorage /* && deviceFeatures.hasTextureFeature(TextureFeatures::ColorTexStorageLa8)*/) ||
                (texImage /* && deviceFeatures.hasTextureFeature(TextureFeatures::ColorTexImageLa8)*/))
            {
                internalFormat = GL_LUMINANCE8;
            }
            return true;

        case TextureFormat::R_UNorm8:
            format = GL_RED;
            type = GL_UNSIGNED_BYTE;
            internalFormat = GL_R8;
            if (texImage /* && !deviceFeatures.hasTextureFeature(TextureFeatures::ColorTexImageRg8)*/)
            {
                internalFormat = GL_RED;
            }
            return true;

        case TextureFormat::R_F16:
            format = GL_RED;
            //            if (deviceFeatures.hasInternalRequirement(InternalRequirement::TextureHalfFloatExtReq)) {
            //                type = GL_HALF_FLOAT_OES; // NOTE: NOT the same as GL_HALF_FLOAT
            //            } else {
            type = GL_HALF_FLOAT;
            //            }
            internalFormat = GL_R16F;
            if (texImage /*&& !deviceFeatures.hasTextureFeature(TextureFeatures::ColorTexImage16f)*/)
            {
                internalFormat = GL_RED;
            }
            return true;

        case TextureFormat::R_F32:
            format = GL_RED;
            type = GL_FLOAT;
            internalFormat = GL_R32F;
            if (texImage /*&& !deviceFeatures.hasTextureFeature(TextureFeatures::ColorTexImage32f)*/)
            {
                internalFormat = GL_RED;
            }
            return true;

        case TextureFormat::R_UInt16:
            internalFormat = GL_R16UI;
            format = GL_RED_INTEGER;
            type = GL_UNSIGNED_SHORT;
            return true;

        case TextureFormat::R_UNorm16:
            internalFormat = GL_R16;
            format = GL_RED;
            type = GL_UNSIGNED_SHORT;
            return true;

        case TextureFormat::RGBA_ASTC_4x4:
            internalFormat = GL_COMPRESSED_RGBA_ASTC_4x4_KHR;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::SRGB8_A8_ASTC_4x4:
            internalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::RGBA_ASTC_5x4:
            internalFormat = GL_COMPRESSED_RGBA_ASTC_5x4_KHR;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::SRGB8_A8_ASTC_5x4:
            internalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::RGBA_ASTC_5x5:
            internalFormat = GL_COMPRESSED_RGBA_ASTC_5x5_KHR;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::SRGB8_A8_ASTC_5x5:
            internalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::RGBA_ASTC_6x5:
            internalFormat = GL_COMPRESSED_RGBA_ASTC_6x5_KHR;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::SRGB8_A8_ASTC_6x5:
            internalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::RGBA_ASTC_6x6:
            internalFormat = GL_COMPRESSED_RGBA_ASTC_6x6_KHR;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::SRGB8_A8_ASTC_6x6:
            internalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::RGBA_ASTC_8x5:
            internalFormat = GL_COMPRESSED_RGBA_ASTC_8x5_KHR;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::SRGB8_A8_ASTC_8x5:
            internalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::RGBA_ASTC_8x6:
            internalFormat = GL_COMPRESSED_RGBA_ASTC_8x6_KHR;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::SRGB8_A8_ASTC_8x6:
            internalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::RGBA_ASTC_8x8:
            internalFormat = GL_COMPRESSED_RGBA_ASTC_8x8_KHR;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::SRGB8_A8_ASTC_8x8:
            internalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::RGBA_ASTC_10x5:
            internalFormat = GL_COMPRESSED_RGBA_ASTC_10x5_KHR;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::SRGB8_A8_ASTC_10x5:
            internalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::RGBA_ASTC_10x6:
            internalFormat = GL_COMPRESSED_RGBA_ASTC_10x6_KHR;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::SRGB8_A8_ASTC_10x6:
            internalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::RGBA_ASTC_10x8:
            internalFormat = GL_COMPRESSED_RGBA_ASTC_10x8_KHR;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::SRGB8_A8_ASTC_10x8:
            internalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::RGBA_ASTC_10x10:
            internalFormat = GL_COMPRESSED_RGBA_ASTC_10x10_KHR;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::SRGB8_A8_ASTC_10x10:
            internalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::RGBA_ASTC_12x10:
            internalFormat = GL_COMPRESSED_RGBA_ASTC_12x10_KHR;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::SRGB8_A8_ASTC_12x10:
            internalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::RGBA_ASTC_12x12:
            internalFormat = GL_COMPRESSED_RGBA_ASTC_12x12_KHR;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::SRGB8_A8_ASTC_12x12:
            internalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::RGBA_BC7_UNORM_4x4:
            internalFormat = GL_COMPRESSED_RGBA_BPTC_UNORM;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::RGBA_PVRTC_2BPPV1:
            internalFormat = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::RGB_PVRTC_2BPPV1:
            internalFormat = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::RGBA_PVRTC_4BPPV1:
            internalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::RGB_PVRTC_4BPPV1:
            internalFormat = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::RGB8_ETC1:
            internalFormat = GL_ETC1_RGB8_OES;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::RGB8_ETC2:
            internalFormat = GL_COMPRESSED_RGB8_ETC2;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::RGB8_Punchthrough_A1_ETC2:
            internalFormat = GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::RGBA8_EAC_ETC2:
            internalFormat = GL_COMPRESSED_RGBA8_ETC2_EAC;
            format = GL_RGBA;
            type = 0;
            return compressedValid;

        case TextureFormat::SRGB8_ETC2:
            internalFormat = GL_COMPRESSED_SRGB8_ETC2;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::SRGB8_Punchthrough_A1_ETC2:
            internalFormat = GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::SRGB8_A8_EAC_ETC2:
            internalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::RG_EAC_UNorm:
            internalFormat = GL_COMPRESSED_RG11_EAC;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::RG_EAC_SNorm:
            internalFormat = GL_COMPRESSED_SIGNED_RG11_EAC;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::R_EAC_UNorm:
            internalFormat = GL_COMPRESSED_R11_EAC;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::R_EAC_SNorm:
            internalFormat = GL_COMPRESSED_SIGNED_R11_EAC;
            format = 0;
            type = 0;
            return compressedValid;

        case TextureFormat::S8_UInt_Z32_UNorm:
            // TODO: Fix this texture type. No backend has a 32-bit int depth + 8-bit int stencil.
            internalFormat = GL_DEPTH32F_STENCIL8;
            format = GL_DEPTH_STENCIL;
            type = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
            return true;

        case TextureFormat::S_UInt8:
            // TODO: test this
            internalFormat = GL_STENCIL_INDEX8;
            format = GL_STENCIL_INDEX;
            type = GL_UNSIGNED_BYTE;
            return true;

        case TextureFormat::Z_UNorm16:
            format = GL_DEPTH_COMPONENT;
            type = GL_UNSIGNED_SHORT;
            internalFormat = GL_DEPTH_COMPONENT16;
            if (texImage /*&& !deviceFeatures.hasTextureFeature(TextureFeatures::DepthTexImage16)*/)
            {
                internalFormat = GL_DEPTH_COMPONENT;
            }
            return true;

        case TextureFormat::Z_UNorm32:
            format = GL_DEPTH_COMPONENT;
            type = GL_UNSIGNED_INT;
            internalFormat = GL_DEPTH_COMPONENT32;
            if (texImage /*&& deviceFeatures.hasInternalRequirement(InternalRequirement::Depth32Unsized)*/)
            {
                internalFormat = GL_DEPTH_COMPONENT;
            }
            return true;

        case TextureFormat::Z_UNorm24:
            format = GL_DEPTH_COMPONENT;
            type = GL_UNSIGNED_INT;
            internalFormat = GL_DEPTH_COMPONENT24;
            return true;

        case TextureFormat::S8_UInt_Z24_UNorm:
            // Support for TextureBuffer and renderbuffer introduced with the same versions / extensions
            format = GL_DEPTH_STENCIL;
            type = GL_UNSIGNED_INT_24_8;
            internalFormat = GL_DEPTH24_STENCIL8;
            if (texImage /*&&
                deviceFeatures.hasInternalRequirement(InternalRequirement::Depth24Stencil8Unsized)*/
            )
            {
                internalFormat = GL_DEPTH_STENCIL;
            }
            return true;
    }

    return false;
}
TextureRangeDesc Texture::getFullRange(size_t mipLevel, size_t numMipLevels_) const
{
    static constexpr size_t one = 1;
    const auto texWidth = std::max(getWidth() >> mipLevel, one);
    const auto texHeight = std::max(getHeight() >> mipLevel, one);
    const auto texDepth = std::max(getDepth() >> mipLevel, one);

    auto desc = TextureRangeDesc::new3D(0, 0, 0, texWidth, texHeight, texDepth, mipLevel);
    desc.numLayers = getNumLayers();
    desc.numMipLevels = numMipLevels_;

    return desc;
}

GLint Texture::getAlignment(size_t stride, size_t mipLevel) const
{
    if (formatProperties.isCompressed())
    {
        return 1;
    }

    const auto srcWidth = std::max(getWidth() >> mipLevel, static_cast<size_t>(1));
    const auto pixelBytesPerRow = formatProperties.getBytesPerRow(srcWidth);

    if (stride == 0 || pixelBytesPerRow > stride)
    {
        return 1;
    }
    else if (stride % 8 == 0)
    {
        return 8;
    }
    else if (stride % 4 == 0)
    {
        return 4;
    }
    else if (stride % 2 == 0)
    {
        return 2;
    }
    else
    {
        return 1;
    }
}

std::pair<bool, bool> Texture::validateRange(const TextureRangeDesc& range) const
{
    const auto texWidth = std::max(getWidth() >> range.mipLevel, static_cast<size_t>(1));
    const auto texHeight = std::max(getHeight() >> range.mipLevel, static_cast<size_t>(1));
    const auto texDepth = std::max(getDepth() >> range.mipLevel, static_cast<size_t>(1));
    const auto texLayers = getNumLayers();

    if (range.x >= texWidth || range.y >= texHeight || range.z >= texDepth)
    {
        return {false, false};
    }

    if (range.width == 0 || range.height == 0 || range.depth == 0)
    {
        return {false, false};
    }

    if (range.x + range.width > texWidth || range.y + range.height > texHeight || range.z + range.depth > texDepth)
    {
        return {false, false};
    }

    if (range.numLayers == 0 || range.numLayers > texLayers)
    {
        return {false, false};
    }

    if (range.numMipLevels == 0 || range.numMipLevels > getNumMipLevels())
    {
        return {false, false};
    }

    const bool fullRange = (range.x == 0 && range.y == 0 && range.z == 0 && range.layer == 0 &&
                            range.width == texWidth && range.height == texHeight &&
                            range.depth == texDepth && range.numLayers == texLayers);

    return {true, fullRange};
}

float Texture::getAspectRatio() const
{
    return static_cast<float>(getWidth()) / static_cast<float>(getHeight());
}

size_t Texture::getWidth() const
{
    return width;
}

size_t Texture::getHeight() const
{
    return height;
}

size_t Texture::getDepth() const
{
    return depth;
}

size_t Texture::getNumLayers() const
{
    return numLayers;
}

size_t Texture::getSamples() const
{
    return numSamples;
}

size_t Texture::getNumMipLevels() const
{
    return numMipLevels;
}

bool Texture::isRequiredGenerateMipmap() const
{
    return false;
}

TextureFormatProperties Texture::getProperties() const
{
    return formatProperties;
}

TextureType Texture::getType() const
{
    return type;
}

void Texture::generateMipmap() const
{
}

TextureFormat Texture::getFormat() const
{
    return formatProperties.format;
}

size_t Texture::getSamplerHash() const
{
    return samplerHash;
}

void Texture::setSamplerHash(size_t hash)
{
    samplerHash = hash;
}

GLenum Texture::getTextureTarget(TextureType type, bool isMultisampled)
{
    switch (type)
    {
        case TextureType::Texture2D:
        {
            if (isMultisampled)
            {
                return GL_TEXTURE_2D_MULTISAMPLE;
            }
            return GL_TEXTURE_2D;
        }
        case TextureType::Texture2DArray:
        {
            if (isMultisampled)
            {
                return GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
            }
            return GL_TEXTURE_2D_ARRAY;
        }
        case TextureType::Texture3D:
        {
            return GL_TEXTURE_3D;
        }
        case TextureType::TextureCube:
        {
            return GL_TEXTURE_CUBE_MAP;
        }
        default:
        {
            return 0;
        }
    }
}


}// namespace opengl
