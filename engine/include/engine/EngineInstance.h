//
// Created by Jonathan Richard on 2024-02-09.
//

#pragma once

#include "Camera.h"
#include "Input.h"
#include "Mesh.h"
#include "MeshRenderer.h"
#include "Model.h"
#include "ResourceManager.h"
#include "SceneRenderer.h"
#include "Stage.h"
#include "TestRenderPass.h"
#include "engine/graphics/Renderer.h"

struct InstanceDesc
{
    std::string assetPath;
    int width;
    int height;
};

class EngineInstance
{
    friend class SceneEditor;
public:
    explicit EngineInstance(InstanceDesc desc);
    ~EngineInstance();

    void updateDisplay(int width, int height);

    void initialize();
    void updateSimulation(float dt);
    void renderFrame();
    void shutdown();

    graphics::Renderer& getRenderer();
    Stage& getStage();
    Input& getInput();

private:
    Input input;

    graphics::Renderer renderer;

    ResourceManager resourceManager;

    TestRenderPass testRenderPass;

    InstanceDesc desc;

    std::shared_ptr<Camera> activeCamera;

    Stage stage;
    std::shared_ptr<Scene> defaultScene;

    std::shared_ptr<ITexture> testRenderTexture;

    SceneRenderer sceneRenderer;

    std::shared_ptr<graphics::Material> normalMaterial;
    std::shared_ptr<graphics::Material> testMaterial;
    std::shared_ptr<graphics::Material> floorMaterial;
    std::shared_ptr<graphics::Material> portalMaterial;

    float pitch = 0.0f;
    float yaw = 0.0f;
    float roll = 0.0f;
};