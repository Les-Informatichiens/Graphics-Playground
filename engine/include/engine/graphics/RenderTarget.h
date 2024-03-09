//
// Created by Jonathan Richard on 2024-03-05.
//

#pragma once


#include "graphicsAPI/common/Common.h"
#include "graphicsAPI/common/Texture.h"

#include <memory>

namespace graphics {

struct RenderTarget
{
    Viewport viewport = Viewport(0, 0, 1, 1);
    std::shared_ptr<ITexture> colorTexture = nullptr;
    Color clearColor = Color(0.0f, 0.0f, 0.0f, 1.0f);
    bool clear = true;
};

}// namespace graphics
