//
// Created by Jonathan Richard on 2024-02-09.
//

#include "engine/EngineInstance.h"
#include "engine/MeshRenderer.h"
#include <iostream>
#include <utility>
#include "engine/EntityView.h"
#include "engine/OBJ_Loader.h"

EngineInstance::EngineInstance(InstanceDesc desc)
    : desc(std::move(desc)), renderer(), stage(), sceneRenderer()
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

    // Create a scene
    defaultScene = std::make_shared<Scene>();
    {
        EntityView root = defaultScene->createEntity("rootNode");
        auto& rootNode = root.getComponent<SceneNode>();
        rootNode.setMesh(m);
        rootNode.getTransform().setPosition({0.0f, 0.0f, 0.0f});
        rootNode.getTransform().setScale({1.f, 1.f, 1.f});
        rootNode.getTransform().setRotation({0.0f, 0.0f, 0.0f});

        // Create a child node
        EntityView child = defaultScene->createEntity("childNode");
        auto& childNode = child.getComponent<SceneNode>();
        childNode.setMesh(m);
        childNode.getTransform().setPosition({-7.0f, 0.0f, 0.0f});
        childNode.getTransform().setScale({1.f, 1.f, 1.f});
        childNode.getTransform().setRotation({0.0f, 0.0f, 0.0f});
        rootNode.addChild(&childNode);
    }

    stage.setScene(defaultScene);
}

void EngineInstance::updateSimulation(float dt)
{
//    std::cout << "Updating simulation (" << (dt * 1000.0f) << " ms)" << std::endl;

    // We can move things around in our testing scene
    {
        std::optional<EntityView> root_ = defaultScene->getEntityByName("rootNode");
        if (root_)
        {
            auto& root = root_.value().getComponent<SceneNode>();
            root.getTransform().rotate(glm::angleAxis(glm::radians(1.0f), glm::vec3(0.0f, 0.0f, 1.0f)));

            auto* childNode_ = root.findNode("childNode");
            if (childNode_)
            {
                auto& childNode = *childNode_;
                childNode.getTransform().rotate(glm::angleAxis(glm::radians(4.0f), glm::vec3(0.0f, 1.0f, -1.0f)));
            }
        }
    }

    stage.update(dt);
}

void EngineInstance::renderFrame()
{
    if (!renderer.isInitialized())
    {
        // log error
        return;
    }

    renderer.begin();

    if (auto* activeScene = stage.getScene())
    {
        SceneRenderData sceneRenderData;
        activeScene->getSceneRenderData(sceneRenderData);
        sceneRenderer.render(renderer, sceneRenderData);
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