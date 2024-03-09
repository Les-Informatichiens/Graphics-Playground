//
// Created by jeang on 2024-01-25.
//
#pragma once

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include "engine/EngineInstance.h"
#include "imgui/ImGuiInstance.h"
#include "imgui/drawingPanel/picasso.h"
#include "imgui/drawingPanel/shape.h"
#include "imgui/scenePanel/SceneEditor.h"

class application
{
public:

    application(EngineInstance& engine, GLFWwindow *window)
        : gameEngine(engine), window(window), imguiInstance({}), sceneEditor(engine) {};

    void init();

    void onWindowResize(int width, int height);
    void onKey(int key, int scancode, int action, int mods);
    void onMouseButton(int button, int action, int mods);
    void onMouseMove(double xpos, double ypos);
    void onMouseScroll(double xoffset, double yoffset);


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
    bool show_demo_window = true;
    bool show_another_window = false;
    bool show_editor = false;
    bool show_pov_cam = false;
    bool lockCamOnSelected = true;
    bool cameraMotion = true;

    picasso vectorDrawer;
    SceneEditor sceneEditor;
};