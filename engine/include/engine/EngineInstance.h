//
// Created by Jonathan Richard on 2024-02-09.
//

#include "Camera.h"
#include "Mesh.h"
#include "Model.h"
#include "TestRenderPass.h"
#include "engine/graphics/Renderer.h"

struct InstanceDesc
{
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

    void drawNode(const std::unique_ptr<SceneNode>& node);

    graphics::Renderer& getRenderer();

private:
    graphics::Renderer renderer;
    TestRenderPass testRenderPass;

    InstanceDesc desc;

    Model testModel;

    std::shared_ptr<Camera> activeCamera;
    std::unique_ptr<SceneNode> root;
};