//
// Created by jeang on 2024-01-25.
//
#pragma once

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include <GL/glew.h>

#include "engine/EngineInstance.h"
#include "imgui/ImGuiInstance.h"
#include "imgui/drawingPanel/picasso.h"
#include "imgui/drawingPanel/shape.h"
#include "imgui/scenePanel/SceneEditor.h"

#include "graphicsAPI/common/Texture.h"

#include <memory>


struct PixelColor
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};
struct ImageData
{
    int w;
    int h;
    int comp;
    int originalComp;
    unsigned char* pixels;

    PixelColor getPixel(int x, int y)
    {
        int i = y * this->w + x;
        return PixelColor { .r = this->pixels[i], .g = this->pixels[i + 1], .b = this->pixels[i + 2], .a = this->pixels[i + 3] };
    }


    void setPixel(int x, int y, PixelColor color)
    {
        int index = (y * w + x) * comp;
        pixels[index] = color.r;
        pixels[index + 1] = color.g;
        pixels[index + 2] = color.b;
        if (comp == 4)
            pixels[index + 3] = color.a;
    }
};

class application
{
public:

    application(EngineInstance& engine, GLFWwindow *window)
        : gameEngine(engine), window(window), imguiInstance({}), sceneEditor(engine), imageData({}), imageTexture(nullptr) {};

    void init();

    void onWindowResize(int width, int height);
    void onKey(int key, int scancode, int action, int mods);
    void onMouseButton(int button, int action, int mods);
    void onMouseMove(double xpos, double ypos);
    void onMouseScroll(double xoffset, double yoffset);


    void run();

    void initImGui();
    void beginImGuiFrame();
    void endImGuiFrame();
    void renderImGuiFrame();
    void shutdownImGui();

    ~application();

private:

    EngineInstance& gameEngine;
    imgui::ImGuiInstance imguiInstance;
    GLFWwindow* window;
    bool windowShouldClose = false;

    int width{};
    int height{};

    // imgui state
    bool show_demo_window = true;
    bool show_another_window = false;
    bool show_editor = false;
    bool show_pov_cam = false;
    bool lockCamOnSelected = true;
    bool cameraMotion = true;
    bool showImageWindow = false;

    picasso vectorDrawer;

    std::string selectedImagePath;
    ImageData imageData;
    std::shared_ptr<ITexture> imageTexture;
    void calculateAndDisplayHistogram();
    void histogram();
    SceneEditor sceneEditor;
};