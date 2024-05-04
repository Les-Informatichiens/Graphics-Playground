//
// Created by Jonathan Richard on 2024-01-29.
//
extern "C" {
__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#include "application.h"
#define TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#include "../src/imgui/raytracerPanel/object_loader/tiny_obj_loader.h"

int main(int argc, char* argv[])
{
    // Window init
    constexpr int width = 1920;
    constexpr int height = 1080;

    if (!glfwInit())
        return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(width, height, "ENGINFORMATICHIENS", nullptr, nullptr);
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
    const InstanceDesc desc = {
            .assetPath = "./assets",
            .width = width,
            .height = height
    };
    EngineInstance engine(desc);

    // Application init
    application app(engine, window);
    app.init();

    app.run();
    return 0;
}