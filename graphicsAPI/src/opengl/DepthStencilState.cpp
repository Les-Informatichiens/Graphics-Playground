//
// Created by Jonathan Richard on 2024-02-08.
//

#include "DepthStencilState.h"

namespace opengl {

static GLenum toOpenGLCompareOp(CompareOp op)
{
    switch (op)
    {
    case CompareOp::Never:
        return GL_NEVER;
    case CompareOp::Less:
        return GL_LESS;
    case CompareOp::Equal:
        return GL_EQUAL;
    case CompareOp::LessOrEqual:
        return GL_LEQUAL;
    case CompareOp::Greater:
        return GL_GREATER;
    case CompareOp::NotEqual:
        return GL_NOTEQUAL;
    case CompareOp::GreaterOrEqual:
        return GL_GEQUAL;
    case CompareOp::Always:
        return GL_ALWAYS;
    default:
        return GL_ALWAYS;
    }
}

static GLenum toOpenGLStencilOp(StencilOp op)
{
    switch (op)
    {
    case StencilOp::Keep:
        return GL_KEEP;
    case StencilOp::Zero:
        return GL_ZERO;
    case StencilOp::Replace:
        return GL_REPLACE;
    case StencilOp::IncrementClamp:
        return GL_INCR;
    case StencilOp::DecrementClamp:
        return GL_DECR;
    case StencilOp::Invert:
        return GL_INVERT;
    case StencilOp::IncrementWrap:
        return GL_INCR_WRAP;
    case StencilOp::DecrementWrap:
        return GL_DECR_WRAP;
    default:
        return GL_KEEP;
    }
}

DepthStencilState::DepthStencilState(Context& context, const DepthStencilStateDesc& desc_)
    : WithContext(context), desc(desc_)
{
}

const DepthStencilStateDesc& DepthStencilState::getDepthStencilStateDesc() const
{
    return desc;
}

void DepthStencilState::bind()
{
    getContext().depthMask(desc.depth.writeEnabled);
    if (desc.depth.writeEnabled || desc.depth.compareOp != CompareOp::Always)
    {
        getContext().enable(GL_DEPTH_TEST);
    }
    else
    {
        getContext().disable(GL_DEPTH_TEST);
    }
    getContext().depthFunc(toOpenGLCompareOp(desc.depth.compareOp));

    if (getContext().isEnabled(GL_STENCIL_TEST))
    {
        GLuint mask = 0xFF;
        GLenum frontCompareOp = toOpenGLCompareOp(desc.stencilFront.compareOp);
        GLenum backCompareOp = toOpenGLCompareOp(desc.stencilBack.compareOp);
        getContext().stencilFuncSeparate(GL_FRONT, frontCompareOp, desc.stencilFront.readMask, mask);
        getContext().stencilFuncSeparate(GL_BACK, backCompareOp, desc.stencilBack.readMask, mask);

        GLenum stencilFailOpFront = toOpenGLStencilOp(desc.stencilFront.failOp);
        GLenum depthFailOpFront = toOpenGLStencilOp(desc.stencilFront.depthFailOp);
        GLenum depthPassOpFront = toOpenGLStencilOp(desc.stencilFront.passOp);
        getContext().stencilOpSeparate(GL_FRONT, stencilFailOpFront, depthFailOpFront, depthPassOpFront);

        GLenum stencilFailOpBack = toOpenGLStencilOp(desc.stencilBack.failOp);
        GLenum depthFailOpBack = toOpenGLStencilOp(desc.stencilBack.depthFailOp);
        GLenum depthPassOpBack = toOpenGLStencilOp(desc.stencilBack.passOp);
        getContext().stencilOpSeparate(GL_BACK, stencilFailOpBack, depthFailOpBack, depthPassOpBack);

        getContext().stencilMaskSeparate(GL_FRONT, desc.stencilFront.writeMask);
        getContext().stencilMaskSeparate(GL_BACK, desc.stencilBack.writeMask);
    }

}

}// namespace opengl
