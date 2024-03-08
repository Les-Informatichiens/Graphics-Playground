//
// Created by jeang on 2024-01-25.
//
#pragma once

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include <GL/glew.h>

#include "engine/EngineInstance.h"
#include "imgui/ImGuiInstance.h"

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
    unsigned char* pixels;

    PixelColor getPixel(int x, int y)
    {
        // implement get pixel at position x and y
    }

    void setPixel(int x, int y, PixelColor color)
    {
        // implement set pixel color at position x and y
    }
};

class application
{
public:

    application(EngineInstance& engine, GLFWwindow *window)
        : gameEngine(engine), window(window), imguiInstance({}) {};

    void init();

    void onWindowResize(int width, int height);

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
    double time = 0.0;
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    std::string selectedImagePath;
    ImageData imageData;
    std::shared_ptr<ITexture> imageTexture;
};