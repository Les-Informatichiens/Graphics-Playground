//
// Created by Jonathan Richard on 2024-01-29.
//

#include "graphicsAPI/opengl/Device.h"
#include "application.h"

int main(int argc, char* argv[])
{
    // Window init
    int width = 1280;
    int height = 720;

    GLFWwindow* window;
    if (!glfwInit())
        return -1;
    window = glfwCreateWindow(width, height, "ENGINFORMATICHIENS", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // find a better way to init things this is needed right now to create the opengl context
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    // Renderer init
//    auto oglContext = std::make_unique<opengl::Context>();
//    opengl::Device device(std::move(oglContext));

    // Engine init
    InstanceDesc desc = {
            .assetPath = "./assets",
            .width = width,
            .height = height
    };
    EngineInstance engine(desc);

    // Application init
    application app(engine, window);
    app.init();

    app.run();
}