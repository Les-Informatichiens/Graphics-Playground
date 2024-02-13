//
// Created by Jonathan Richard on 2024-02-12.
//

#pragma once

#include "GLFW/glfw3.h"
#include "imgui.h"

struct ImGuiContextAdapterDesc
{
    GLFWwindow* glfwWindow;
    const char* glslVersion;
};

class ImGuiContextAdapter
{
public:
    explicit ImGuiContextAdapter() = default;
    ~ImGuiContextAdapter() = default;

    void initialize(ImGuiContextAdapterDesc desc);

    void beginFrame() const;
    void endFrame() const;
    void renderFrame() const;

    void shutdown();
private:
    ImGuiContext* context = nullptr;
};