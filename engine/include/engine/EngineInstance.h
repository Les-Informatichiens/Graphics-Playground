//
// Created by Jonathan Richard on 2024-02-09.
//

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
    void shutdown();

    graphics::Renderer& getRenderer();

private:
    graphics::Renderer renderer;

    InstanceDesc desc;
};