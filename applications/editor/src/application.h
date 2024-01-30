//
// Created by jeang on 2024-01-25.
//
#pragma once

#include "GLFW/glfw3.h"
#include "engine/engine.h"

class application
{
public:

    application(const engine& engine, GLFWwindow *window) : engine(engine), window(window) {};

    void init();

    void run();

    ~application();

private:

    const engine& engine;
    GLFWwindow* window;
    bool windowShouldClose = false;
};