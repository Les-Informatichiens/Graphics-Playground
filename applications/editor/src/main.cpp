//
// Created by Jonathan Richard on 2024-01-29.
//

#include "renderer/renderAPI/openGLRenderer.h"
#include "application.h"

int main(int argc, char* argv[])
{
    // Renderer init
    openGLRenderer renderer;

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