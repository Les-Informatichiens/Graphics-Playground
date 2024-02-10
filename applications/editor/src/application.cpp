//
// Created by jeang on 2024-01-25.
//

#include "application.h"
#include "backends/imgui_impl_glfw.h"

//implement the application class here
void application::init()
{
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    });

    // add window resize callback and bind this object's pointer to it
    glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width_, int height_) {
        auto* app = (application*)glfwGetWindowUserPointer(window);
        app->onWindowResize(width_, height_);
    });

    glfwMakeContextCurrent(window);

    ImGui_ImplGlfw_InitForOpenGL(window, true);

    // Initialize the game engine and the rendering context
    gameEngine.initialize();

    glfwGetWindowSize(window, &width, &height);
    imguiInstance.initialize(gameEngine.getRenderer().getDevice(), width, height);

}

void application::run()
{

    while (!windowShouldClose)
    {
        ImGui_ImplGlfw_NewFrame();
        imguiInstance.beginFrame();

        ImGui::ShowDemoWindow();

        ImGui::Begin("Hello, world!");
        ImGui::Button("Look at this pretty button");
        ImGui::Text("Hello, world!");
        ImGui::End();

        gameEngine.updateSimulation(0.0f);

        auto commandPool = gameEngine.getRenderer().getDevice().createCommandPool({});
        auto commandBuffer = commandPool->acquireCommandBuffer({});
        commandBuffer->beginRenderPass({
                .renderPass = {
                        .colorAttachments = {
                                RenderPassDesc::ColorAttachmentDesc{
                                        LoadAction::Clear,
                                        StoreAction::DontCare,
                                        {0.45f, 0.55f, 0.60f, 1.00f}
                                }
                        }
                },
                .framebuffer = nullptr
        });

        imguiInstance.endFrame();
        imguiInstance.renderFrame(gameEngine.getRenderer().getDevice(), commandBuffer, nullptr, width, height);

        commandBuffer->endRenderPass();

        commandPool->submitCommandBuffer(commandBuffer);


        glfwSwapBuffers(window);
        glfwPollEvents();
        windowShouldClose = glfwWindowShouldClose(window);
    }
}

application::~application()
{
    ImGui_ImplGlfw_Shutdown();
    imguiInstance.shutdown();
    gameEngine.shutdown();
    glfwTerminate();
}

void application::onWindowResize(int width, int height)
{
    this->width = width;
    this->height = height;
    gameEngine.updateDisplay(width, height);
}
