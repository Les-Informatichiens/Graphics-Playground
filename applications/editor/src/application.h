//
// Created by jeang on 2024-01-25.
//
#pragma once

#include "engine/engine.h"
#include "imgui/ImGuiInstance.h"
#include <cstdio>

//#define GL_SILENCE_DEPRECATION
//#if defined(IMGUI_IMPL_OPENGL_ES2)
//#include <GLES2/gl2.h>
//#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>// Will drag system OpenGL headers

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif


class application
{
public:
    application(engine& engine) : gameEngine(engine), imguiInstance() {}

    int init();

    void run();

    ~application();

private:
    void endImGuiFrame() const;
    void renderImGuiFrame() const;
    void beginImGuiFrame() const;

private:
    engine& gameEngine;

    ImGuiInstance imguiInstance;

    GLFWwindow* window = nullptr;
    bool windowShouldClose = false;
};
