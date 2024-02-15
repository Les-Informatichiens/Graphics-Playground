//
// Created by Jonathan Richard on 2024-02-09.
//

#include "Camera.h"
#include "Mesh.h"
#include "MeshRenderer.h"
#include "Model.h"
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

    void drawNode(const std::unique_ptr<SceneNode>& node);

    graphics::Renderer& getRenderer();

private:
    graphics::Renderer renderer;
    TestRenderPass testRenderPass;

    // Utility class for rendering plain meshes,
    // it could help to cache and reuse renderable data later
    MeshRenderer meshRenderer{};

    InstanceDesc desc;

    std::shared_ptr<Camera> activeCamera;
    std::unique_ptr<SceneNode> root;
};