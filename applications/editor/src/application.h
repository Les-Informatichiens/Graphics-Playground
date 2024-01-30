//
// Created by jeang on 2024-01-25.
//
#pragma once

#include "engine/engine.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

class application
{
public:

    application(engine engine, GLFWwindow *window) : engine(engine), window(window) {};

    void init();

    void run();

    ~application();

private:

    const engine engine;
    GLFWwindow* window;
    bool windowShouldClose = false;
};