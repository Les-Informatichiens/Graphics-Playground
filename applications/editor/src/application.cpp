//
// Created by jeang on 2024-01-25.
//

#include "application.h"

//implement the application class here
void application::init()
{
    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    });

    glfwMakeContextCurrent(window);
    }

void application::run()
{

    while (!windowShouldClose)
    {
        engine.update(0.0f);
        engine.render();

        glfwSwapBuffers(window);
        glfwPollEvents();
        windowShouldClose = glfwWindowShouldClose(window);
    }
}

application::~application()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}
