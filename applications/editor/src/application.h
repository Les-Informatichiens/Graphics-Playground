//
// Created by jeang on 2024-01-25.
//
#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>// Will drag system OpenGL headers

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif


#include "engine/EngineInstance.h"
#include "imgui/ImGuiInstance.h"

class application
{
public:
    application(engine& engine) : gameEngine(engine), imguiInstance() {}

    application(EngineInstance& engine, GLFWwindow *window)
        : gameEngine(engine), window(window), imguiInstance({}) {};

    void init();

    void onWindowResize(int width, int height);

    void run();

    void initImGui();
    void beginImGuiFrame();
    void endImGuiFrame();
    void renderImGuiFrame();
    void shutdownImGui();

    ~application();

private:
    void endImGuiFrame() const;
    void renderImGuiFrame() const;
    void beginImGuiFrame() const;

    EngineInstance& gameEngine;
    imgui::ImGuiInstance imguiInstance;
    GLFWwindow* window;
    bool windowShouldClose = false;

    int width;
    int height;

    // imgui state
    double time = 0.0;
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
};
