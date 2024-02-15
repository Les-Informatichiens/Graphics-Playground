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
    : desc(desc), renderer()
{
    activeCamera = std::make_unique<Camera>("camera");
}

EngineInstance::~EngineInstance()
{
}

void EngineInstance::updateDisplay(int width, int height)
{
    desc.width = width;
    desc.height = height;

    if (activeCamera)
        activeCamera->setProjectionConfig(45.0f, (float)desc.width / (float)desc.height, 0.1f, 100.0f);
}

void EngineInstance::initialize()
{
    graphics::RendererDesc rendererDesc;
    renderer.initialize(rendererDesc);
    testRenderPass.initialize(renderer.getDevice());

    // ----------- Initializing scene objects ------------

    // Setup camera
    activeCamera->setProjectionConfig(45.0f, (float)desc.width / (float)desc.height, 0.1f, 100.0f);
    activeCamera->getTransform().setPosition({ 0.0f, 0.0f, 20.0f });
    renderer.setCamera(activeCamera);

    // Load teapot model
    std::shared_ptr<Mesh> m = std::make_shared<Mesh>();
    {
        objl::Loader loader;
        bool success = loader.LoadFile(desc.assetPath + "/test/teapot.obj");
        if (!success)
        {
            std::cerr << "Failed to load model" << std::endl;
            return;
        }
        for (auto& LoadedVertice: loader.LoadedVertices)
        {
            Mesh::Vertex vertex{};
            vertex.position = {LoadedVertice.Position.X, LoadedVertice.Position.Y, LoadedVertice.Position.Z};
            vertex.normal = {LoadedVertice.Normal.X, LoadedVertice.Normal.Y, LoadedVertice.Normal.Z};
            m->vertices.push_back(vertex);
        }
        for (unsigned int LoadedIndice: loader.LoadedIndices)
        {
            m->indices.push_back(LoadedIndice);
        }
        m->normalize();
    }

    Model model("teapot");

    // Create a scene
    root = std::make_unique<SceneNode>("rootNode");
    root->setMesh(m);
    root->getTransform().setPosition({ 0.0f, 0.0f, 0.0f });
    root->getTransform().setScale({ 1.f, 1.f, 1.f });
    root->getTransform().setRotation({ 0.0f, 0.0f, 0.0f });

    // Create a child node
    auto child = std::make_unique<SceneNode>("childNode");
    child->setMesh(m);
    child->getTransform().setPosition({ -7.0f, 0.0f, 0.0f });
    child->getTransform().setScale({ 1.f, 1.f, 1.f });
    child->getTransform().setRotation({ 0.0f, 0.0f, 0.0f });
    root->addChild(std::move(child));
}

void EngineInstance::updateSimulation(float dt)
{
//    std::cout << "Updating simulation (" << (dt * 1000.0f) << " ms)" << std::endl;

    // Currently we can update the scene graph nodes by retrieving them and updating their transforms
    // We could set up a better system to store the nodes and update them in a more efficient way
    // without having to traverse the entire scene graph every time we want to change a node's transform
    // but for now this will do.
    // Jonathan Richard 2024-02-14
    if (root)
    {
        root->getTransform().rotate(glm::angleAxis(glm::radians(1.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
    }

    auto* childNode = root->findNode("childNode");
    if (childNode)
    {
        childNode->getTransform().rotate(glm::angleAxis(glm::radians(4.0f), glm::vec3(0.0f, 1.0f, -1.0f)));
    }

    // Update the scene graph
    if (root)
    {
        root->update(dt);
    }
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

    // Draw the scene
    if (root)
    {
        drawNode(root);
    }

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

void EngineInstance::drawNode(const std::unique_ptr<SceneNode>& node)
{
//    std::cout << "Entering DrawNode("<< node->getName() <<")" << std::endl;
    if (node->getMesh())
    {
        node->draw(renderer);
        if (auto mesh = node->getMesh(); mesh)
        {
            meshRenderer.render(renderer, *mesh, node->getWorldTransform());
        }
    }
    for (auto& child : node->getChildren())
    {
        drawNode(child);
    }
//    std::cout << "Done DrawNode("<< node->getName() <<")" << std::endl;
}
