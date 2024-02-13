//
// Created by Jonathan Richard on 2024-02-09.
//

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

    graphics::Renderer& getRenderer();

private:
    graphics::Renderer renderer;
    TestRenderPass testRenderPass;

    InstanceDesc desc;
};