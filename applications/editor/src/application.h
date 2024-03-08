//
// Created by jeang on 2024-01-25.
//
#pragma once

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include "engine/EngineInstance.h"
#include "imgui/ImGuiInstance.h"
#include "imgui/drawingPanel/shape.h"
#include "imgui/drawingPanel/picasso.h"

class application
{
public:

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


    picasso vectorDrawer;
};