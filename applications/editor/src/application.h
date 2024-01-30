//
// Created by jeang on 2024-01-25.
//
#pragma once

#include "GLFW/glfw3.h"
#include "engine/engine.h"

class application
{
public:

    application();

    void init();

    void run();

    bool shouldClose();

    void exit();

private:
    void update();

private:

    engine engine;

    GLFWwindow* window;
    bool windowShouldClose = false;
};