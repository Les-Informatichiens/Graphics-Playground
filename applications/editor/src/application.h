//
// Created by jeang on 2024-01-25.
//
#pragma once

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include "engine/EngineInstance.h"
#include "imgui/ImGuiInstance.h"

class application
{
public:

    application(EngineInstance& engine, GLFWwindow *window)
        : gameEngine(engine), window(window), imguiInstance({}) {};

    void init();

    void onWindowResize(int width, int height);

    void run();

    ~application();

private:

    EngineInstance& gameEngine;
    imgui::ImGuiInstance imguiInstance;
    GLFWwindow* window;
    bool windowShouldClose = false;

    int width;
    int height;
};