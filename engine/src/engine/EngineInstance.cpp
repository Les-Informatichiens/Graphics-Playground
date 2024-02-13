//
// Created by Jonathan Richard on 2024-02-09.
//

#include "engine/EngineInstance.h"
#include "engine/MeshRenderer.h"
#include <iostream>
//#include "assimp/scene.h"
//#include "assimp/postprocess.h"
//#include <assimp/Importer.hpp>
#include "engine/OBJ_Loader.h"

EngineInstance::EngineInstance(InstanceDesc desc)
    : desc(desc), renderer(), camera("camera"), testModel("teapot")
{
}

EngineInstance::~EngineInstance()
{
}

void EngineInstance::updateDisplay(int width, int height)
{
    desc.width = width;
    desc.height = height;

    camera.setProjectionConfig(45.0f, (float)desc.width / (float)desc.height, 0.1f, 100.0f);
}

void EngineInstance::initialize()
{
    graphics::RendererDesc rendererDesc;
    renderer.initialize(rendererDesc);
    testRenderPass.initialize(renderer.getDevice());

    // ----------- Initializing scene objects ------------

    // Setup camera
    camera.setProjectionConfig(45.0f, (float)desc.width / (float)desc.height, 0.1f, 100.0f);
    camera.getTransform().setPosition({ 0.0f, 0.0f, 10.0f });

    // Load teapot model
    objl::Loader loader;
    loader.LoadFile("teapot.obj");
    Mesh m;
    for (auto & LoadedVertice : loader.LoadedVertices)
    {
        Mesh::Vertex vertex{};
        vertex.position = { LoadedVertice.Position.X, LoadedVertice.Position.Y, LoadedVertice.Position.Z };
        vertex.normal = { LoadedVertice.Normal.X, LoadedVertice.Normal.Y, LoadedVertice.Normal.Z };
        m.vertices.push_back(vertex);
    }
    for (unsigned int LoadedIndice : loader.LoadedIndices)
    {
        m.indices.push_back(LoadedIndice);
    }
    m.normalize();

    Model model("teapot");
    model.setMesh(m);
    this->testModel = model;
}

void EngineInstance::updateSimulation(float dt)
{
    std::cout << "Updating simulation (" << (dt * 1000.0f) << " ms)" << std::endl;
    testModel.getTransform().rotate({ 0.06f, 0.25f, 0.1f });
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
//    testRenderPass.render(renderer.getDevice());

    renderer.begin();

    MeshRenderer meshRenderer;

    meshRenderer.render(renderer, testModel.getMesh(), testModel.getTransform(), camera);

    renderer.end();
}

void EngineInstance::shutdown()
{
    renderer.shutdown();
}

graphics::Renderer& EngineInstance::getRenderer()
{
    return renderer;
}