//
// Created by Jonathan Richard on 2024-02-09.
//

#include "SamplerState.h"

#include "DepthStencilState.h"
#include "Texture.h"

namespace opengl {

namespace {
bool isPowerOfTwo(size_t number) {
    return (number & (number - 1)) == 0;
}
} // namespace

SamplerState::SamplerState(Context& context, const SamplerStateDesc& desc)
    : WithContext(context),
      minMipFilter_(convertMinMipFilter(desc.minFilter, desc.mipFilter)),
      magFilter_(convertMagFilter(desc.magFilter)),
      mipLodMin_(desc.mipLodMin),
      mipLodMax_(desc.mipLodMax),
      addressU_(convertAddressMode(desc.addressModeU)),
      addressV_(convertAddressMode(desc.addressModeV)),
      addressW_(convertAddressMode(desc.addressModeW)),
      depthCompareFunction_(DepthStencilState::toOpenGLCompareOp(desc.depthCompareFunction)),
      depthCompareEnabled_(desc.depthCompareEnabled)
{
    (void) mipLodMin_;
    (void) mipLodMax_;
    std::hash<SamplerStateDesc> h;
    hash_ = h(desc);
}

void SamplerState::bind(ITexture* t) {
    if (t == nullptr) {
        return;
    }

    auto texture = static_cast<opengl::Texture*>(t);
    if (texture->getSamplerHash() == hash_) {
        return;
    }
    texture->setSamplerHash(hash_);

    auto type = texture->getType();
    auto target = Texture::getTextureTarget(type, texture->getSamples() > 1);
    if (target == 0) {
        return;
    }

    // From OpenGL ES 3.1 spec
    // The effective internal format specified for the texture arrays is a sized internal depth or
    // depth and stencil format (see table 8.14), the value of TEXTURE_COMPARE_MODE is NONE , and
    // either the magnification filter is not NEAREST or the minification filter is neither
    // NEAREST nor NEAREST_MIPMAP_NEAREST.
    if (!depthCompareEnabled_ && texture->getProperties().isDepthOrStencil() &&
        minMipFilter_ != GL_NEAREST && minMipFilter_ != GL_NEAREST_MIPMAP_NEAREST) {
        auto supportedMode = GL_NEAREST;
        if (minMipFilter_ == GL_LINEAR_MIPMAP_NEAREST || minMipFilter_ == GL_NEAREST_MIPMAP_LINEAR ||
            minMipFilter_ == GL_LINEAR_MIPMAP_LINEAR) {
            supportedMode = GL_NEAREST_MIPMAP_NEAREST;
        }
        getContext().texParameteri(target, GL_TEXTURE_MIN_FILTER, supportedMode);
    } else {
        getContext().texParameteri(target, GL_TEXTURE_MIN_FILTER, minMipFilter_);
    }
    if (!depthCompareEnabled_ && texture->getProperties().isDepthOrStencil() &&
        magFilter_ != GL_NEAREST) {
        getContext().texParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    } else {
        getContext().texParameteri(target, GL_TEXTURE_MAG_FILTER, magFilter_);
    }

    // See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glTexParameter.xml
    // for OpenGL version information.
//    if (deviceFeatures.hasFeature(DeviceFeatures::SamplerMinMaxLod)) {
        getContext().texParameteri(target, GL_TEXTURE_MIN_LOD, mipLodMin_);
        getContext().texParameteri(target, GL_TEXTURE_MAX_LOD, mipLodMax_);
//    }

//    if (deviceFeatures.hasInternalFeature(InternalFeatures::TextureCompare)) {
        getContext().texParameteri(target,
                                   GL_TEXTURE_COMPARE_MODE,
                                   depthCompareEnabled_ ? GL_COMPARE_REF_TO_TEXTURE : GL_NONE);
        getContext().texParameteri(target, GL_TEXTURE_COMPARE_FUNC, depthCompareFunction_);
//    }

//    if (!deviceFeatures.hasFeature(DeviceFeatures::TextureNotPot)) {
        if (!isPowerOfTwo(texture->getWidth()) || !isPowerOfTwo(texture->getHeight())) {
            getContext().texParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            getContext().texParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        } else {
            getContext().texParameteri(target, GL_TEXTURE_WRAP_S, addressU_);
            getContext().texParameteri(target, GL_TEXTURE_WRAP_T, addressV_);
        }
//    } else {
//        getContext().texParameteri(target, GL_TEXTURE_WRAP_S, addressU_);
//        getContext().texParameteri(target, GL_TEXTURE_WRAP_T, addressV_);
//    }

    if (type == TextureType::Texture2DArray || type == TextureType::Texture3D) {
        getContext().texParameteri(target, GL_TEXTURE_WRAP_R, addressW_);
    }
}

// utility functions for converting from IGL sampler state enums to GL enums
GLint SamplerState::convertMinMipFilter(SamplerMinMagFilter minFilter, SamplerMipFilter mipFilter) {
    GLint glMinFilter;

    switch (mipFilter) {
        case SamplerMipFilter::Disabled:
            glMinFilter = (minFilter == SamplerMinMagFilter::Nearest) ? GL_NEAREST : GL_LINEAR;
            break;

        case SamplerMipFilter::Nearest:
            glMinFilter = (minFilter == SamplerMinMagFilter::Nearest) ? GL_NEAREST_MIPMAP_NEAREST
                                                                      : GL_LINEAR_MIPMAP_NEAREST;
            break;

        case SamplerMipFilter::Linear:
            glMinFilter = (minFilter == SamplerMinMagFilter::Nearest) ? GL_NEAREST_MIPMAP_LINEAR
                                                                      : GL_LINEAR_MIPMAP_LINEAR;
            break;
    }

    return glMinFilter;
}

GLint SamplerState::convertMagFilter(SamplerMinMagFilter magFilter) {
    return (magFilter == SamplerMinMagFilter::Nearest) ? GL_NEAREST : GL_LINEAR;
}

SamplerMinMagFilter SamplerState::convertGLMagFilter(GLint glMagFilter) {
    return (glMagFilter == GL_NEAREST) ? SamplerMinMagFilter::Nearest : SamplerMinMagFilter::Linear;
}

SamplerMinMagFilter SamplerState::convertGLMinFilter(GLint glMinFilter) {
    SamplerMinMagFilter minFilter;

    switch (glMinFilter) {
        case GL_NEAREST:
        case GL_NEAREST_MIPMAP_NEAREST:
        case GL_NEAREST_MIPMAP_LINEAR:
            minFilter = SamplerMinMagFilter::Nearest;
            break;

        case GL_LINEAR:
        case GL_LINEAR_MIPMAP_NEAREST:
        case GL_LINEAR_MIPMAP_LINEAR:
            minFilter = SamplerMinMagFilter::Linear;
            break;

        default:
            minFilter = SamplerMinMagFilter::Nearest;
    }

    return minFilter;
}

SamplerMipFilter SamplerState::convertGLMipFilter(GLint glMinFilter) {
    SamplerMipFilter mipFilter;

    switch (glMinFilter) {
        case GL_NEAREST:
        case GL_LINEAR:
            mipFilter = SamplerMipFilter::Disabled;
            break;

        case GL_NEAREST_MIPMAP_NEAREST:
        case GL_LINEAR_MIPMAP_NEAREST:
            mipFilter = SamplerMipFilter::Nearest;
            break;

        case GL_NEAREST_MIPMAP_LINEAR:
        case GL_LINEAR_MIPMAP_LINEAR:
            mipFilter = SamplerMipFilter::Linear;
            break;

        default:
            mipFilter = SamplerMipFilter::Disabled;
    }

    return mipFilter;
}

GLint SamplerState::convertAddressMode(SamplerAddressMode addressMode) {
    GLint glAddressMode;

    switch (addressMode) {
        case SamplerAddressMode::Repeat:
            glAddressMode = GL_REPEAT;
            break;

        case SamplerAddressMode::Clamp:
            glAddressMode = GL_CLAMP_TO_EDGE;
            break;

        case SamplerAddressMode::MirrorRepeat:
            glAddressMode = GL_MIRRORED_REPEAT;
            break;
    }

    return glAddressMode;
}

SamplerAddressMode SamplerState::convertGLAddressMode(GLint glAddressMode) {
    SamplerAddressMode addressMode;

    switch (glAddressMode) {
        case GL_REPEAT:
            addressMode = SamplerAddressMode::Repeat;
            break;

        case GL_CLAMP_TO_EDGE:
            addressMode = SamplerAddressMode::Clamp;
            break;

        case GL_MIRRORED_REPEAT:
            addressMode = SamplerAddressMode::MirrorRepeat;
            break;

        default:
            addressMode = SamplerAddressMode::Repeat;
            break;
    }

    return addressMode;
}

}// namespace opengl
