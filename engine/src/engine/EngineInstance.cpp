//
// Created by Jonathan Richard on 2024-02-09.
//

#include "engine/EngineInstance.h"

EngineInstance::EngineInstance(InstanceDesc desc)
    : desc(desc), renderer()
{
}

EngineInstance::~EngineInstance()
{
}

void EngineInstance::updateDisplay(int width, int height)
{
    desc.width = width;
    desc.height = height;
}

void EngineInstance::initialize()
{
    graphics::RendererDesc rendererDesc;
    renderer.initialize(rendererDesc);
}

void EngineInstance::updateSimulation(float dt)
{
    if (renderer.isInitialized())
    {
        renderer.render();
    }
}

void EngineInstance::shutdown()
{
    renderer.shutdown();
}

graphics::Renderer& EngineInstance::getRenderer()
{
    return renderer;
}
