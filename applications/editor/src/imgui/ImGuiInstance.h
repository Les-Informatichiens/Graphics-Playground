//
// Created by Jonathan Richard on 2024-02-12.
//

#pragma once

#include "GLFW/glfw3.h"
#include "imgui.h"

struct ImGuiInstanceDesc
{
    GLFWwindow* glfwWindow;
    const char* glslVersion;
};

class ImGuiInstance
{
public:
    explicit ImGuiInstance() = default;
    ~ImGuiInstance() = default;

    void initialize(ImGuiInstanceDesc desc);

    void beginFrame() const;
    void endFrame() const;
    void renderFrame() const;

    void shutdown();
private:
    ImGuiContext* context = nullptr;
};