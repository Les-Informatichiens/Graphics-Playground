//
// Created by jeang on 2024-01-25.
//

#include "application.h"

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

//implement the application class here
int application::init()
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 0;
// Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);// 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);          // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
    if (window == nullptr)
        return 0;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);// Enable vsync

    // Initialize ImGui
    imguiInstance.initialize({
        .glfwWindow = window,
        .glslVersion = glsl_version
    });

    return 1;
}

void application::run()
{
    while (!windowShouldClose)
    {
        glfwPollEvents();

        beginImGuiFrame();

        ImGui::ShowDemoWindow();// Show demo window! :)

        // Main Rendering
        gameEngine.update(0.0f);
        gameEngine.render();


        endImGuiFrame();
        renderImGuiFrame();

        glfwSwapBuffers(window);
        windowShouldClose = glfwWindowShouldClose(window);
    }
}

void application::beginImGuiFrame() const
{
    imguiInstance.beginFrame();
}

void application::endImGuiFrame() const
{
    imguiInstance.endFrame();
}

void application::renderImGuiFrame() const
{
    imguiInstance.renderFrame();

    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

application::~application()
{
    imguiInstance.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}
