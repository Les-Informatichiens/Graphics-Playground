//
// Created by Jonathan Richard on 2024-01-29.
//

#include "graphicsAPI/opengl/Device.h"
#include "application.h"

int main(int argc, char* argv[])
{
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
    // find a better way to init things this is needed right now to create the opengl context
    glfwMakeContextCurrent(window);

    // Renderer init
    auto oglContext = std::make_unique<opengl::Context>();
    opengl::Device device(std::move(oglContext));

    // Engine init
    engine engine(device);

    // Application init
    application app(engine, window);
    app.init();

    app.run();
}