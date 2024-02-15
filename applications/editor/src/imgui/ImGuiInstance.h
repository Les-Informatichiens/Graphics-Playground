//
// Created by Jonathan Richard on 2024-02-09.
//

#pragma once

#include "ImGuiRenderer.h"

namespace imgui {

struct ImGuiInstanceDesc
{
};

class ImGuiInstance
{
public:
    explicit ImGuiInstance(ImGuiInstanceDesc desc);
    ~ImGuiInstance();

    void initialize(IDevice& device, uint32_t width, uint32_t height);

    void beginFrame();
    void endFrame();
    void renderFrame(IDevice& device, ICommandBuffer& commandBuffer, std::shared_ptr<IGraphicsPipeline> pipeline, uint32_t fbWidth, uint32_t fbHeight);

    void shutdown();
private:
    ImGuiContext* context;
    ImGuiRenderer renderer;
};

}// namespace imgui
