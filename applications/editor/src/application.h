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
        PixelColor color{};
        int index = (y * w + x) * comp;
        color.r = pixels[index];
        color.g = pixels[index + 1];
        color.b = pixels[index + 2];
        color.a = comp == 4 ? pixels[index + 3] : 255;
        return color;
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

    application(EngineInstance& engine, GLFWwindow* window)
        : gameEngine(engine), window(window), imguiInstance({}), imageData({}), imageTexture(nullptr) {};

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
    void calculateAndDisplayHistogram();
    void histogram();
};