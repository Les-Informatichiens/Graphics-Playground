//
// Created by Jonathan Richard on 2024-01-29.
//

#include "application.h"
#include "renderer/renderAPI/openGLRenderer.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

int main(int argc, char* argv[])
{
    // Renderer init
    openGLRenderer renderer = openGLRenderer();

    // Engine init
    engine engine(renderer);

    // Window init
    GLFWwindow* window;
    if (!glfwInit())
        return -1;
    window = glfwCreateWindow(1280, 720, "ENGINFORMATICHIENS", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // Application init
    application app(engine, window);
    app.init();

    app.run();
}