//
// Created by jeang on 2024-01-25.
//
#pragma once

#include "GLFW/glfw3.h"

class application
{
public:

    void init();

    void update();

    bool shouldClose();

    void exit();

    GLFWwindow* window;
    bool windowShouldClose = false;
};