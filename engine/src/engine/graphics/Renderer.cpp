//
// Created by Jonathan Richard on 2024-02-09.
//

#include "engine/graphics/Renderer.h"
#include "graphicsAPI/opengl/Device.h"

namespace graphics {

void Renderer::initialize(graphics::RendererDesc desc)
{
    // for now we only support opengl
    auto oglContext = std::make_unique<opengl::Context>();

    this->device = std::make_unique<opengl::Device>(std::move(oglContext));

    this->initialized = true;
}

void Renderer::render()
{
}

void Renderer::shutdown()
{
}

IDevice& Renderer::getDevice() const
{
    return *this->device;
}

}// namespace graphics
