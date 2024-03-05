//
// Created by Jonathan Richard on 2024-02-09.
//

#include "Camera.h"
#include "Mesh.h"
#include "MeshRenderer.h"
#include "Model.h"
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

private:
    graphics::Renderer renderer;
    TestRenderPass testRenderPass;

    InstanceDesc desc;

    std::shared_ptr<Camera> activeCamera;

    Stage stage;
    std::shared_ptr<Scene> defaultScene;

    SceneRenderer sceneRenderer;
};