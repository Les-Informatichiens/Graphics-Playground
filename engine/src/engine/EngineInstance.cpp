//
// Created by Jonathan Richard on 2024-02-09.
//

#include "engine/EngineInstance.h"
#include <iostream>

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
    std::cout << "Updating simulation (" << (dt * 1000.0f) << " ms)" << std::endl;
}

void EngineInstance::renderFrame()
{
    if (!renderer.isInitialized())
    {
        // log error
        return;
    }

    // render something for testing purposes
    // Note that this is not the thought out way to render things
    // This is mainly just to render something to the screen and simulate as if the engine was running
    // to then try to render an imgui frame on top of it, all while being decoupled in the code
    // Jonathan Richard 2024-02-10
    testRenderPass.render(renderer.getDevice());

    renderer.render();
}

void EngineInstance::shutdown()
{
    renderer.shutdown();
}

graphics::Renderer& EngineInstance::getRenderer()
{
    return renderer;
}