//
// Created by Jonathan Richard on 2024-02-09.
//

#pragma once

#include "graphicsAPI/common/SamplerState.h"
#include "graphicsAPI/common/Texture.h"
#include "Texture.h"
#include "graphicsAPI/opengl/Context.h"

#include <limits>
#include <memory>

namespace opengl {

class SamplerState final : public WithContext, public ISamplerState
{
public:
    SamplerState(Context& context, const SamplerStateDesc& desc);
    void bind(const std::shared_ptr<Texture>& texture);

    static GLint convertMinMipFilter(SamplerMinMagFilter minFilter, SamplerMipFilter mipFilter);
    static GLint convertMagFilter(SamplerMinMagFilter magFilter);
    static GLint convertAddressMode(SamplerAddressMode addressMode);
    static SamplerAddressMode convertGLAddressMode(GLint glAddressMode);
    static SamplerMinMagFilter convertGLMagFilter(GLint magFilter);
    static SamplerMinMagFilter convertGLMinFilter(GLint minFilter);
    static SamplerMipFilter convertGLMipFilter(GLint minFilter);

private:
    size_t hash_ = std::numeric_limits<size_t>::max();
    GLint minMipFilter_;
    GLint magFilter_;
    GLfloat mipLodMin_;
    GLfloat mipLodMax_;

    GLint addressU_;
    GLint addressV_;
    GLint addressW_;

    GLint depthCompareFunction_;
    bool depthCompareEnabled_;
};

} // namespace opengl