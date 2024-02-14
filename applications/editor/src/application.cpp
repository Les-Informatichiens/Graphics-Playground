//
// Created by jeang on 2024-01-25.
//

#include "application.h"
#include "backends/imgui_impl_glfw.h"
#include <iostream>

//implement the application class here
void application::init()
{
    // Set the user pointer of the window to the application object
    // This is needed so that we can access "this" pointer in glfw callbacks
    glfwSetWindowUserPointer(window, this);

    // The application closes when the escape key is pressed
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

    // The game engine needs to be initialized before the ImGui context,
    // because the ImGui context needs the game engine's renderer to be initialized
    // Jonathan Richard 2024-02-10
    gameEngine.initialize();

    // Initialize the ImGui context
    initImGui();
}

void application::run()
{

    while (!windowShouldClose)
    {
        // We will update the simulation and render the frame before rendering the ImGui frame
        // This is to simulate the game engine running and rendering a frame before the ImGui frame is rendered on top of it
        // Jonathan Richard 2024-02-10
        gameEngine.updateSimulation(0.0f);
        gameEngine.renderFrame();

        beginImGuiFrame();

        // Here we can have some ImGui code that would let the user
        // control some state in the application.
        // We can later put this in a better place, like separate classes
        // that would handle the ImGui code for different parts of the application
        // Jonathan Richard 2024-02-10

        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text."); // Display some text (you can use a format strings too)

            ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            ImGui::End();

            if (show_demo_window)
            {
                ImGui::ShowDemoWindow(&show_demo_window);
            }

            if (show_another_window)
            {
                ImGui::Begin("Another Window", &show_another_window);
                ImGui::Text("Hello from another window!");
                if (ImGui::Button("Close Me"))
                    show_another_window = false;
                ImGui::End();
            }
        }

        // ImGui UI calls should always be done between begin and end frame
        endImGuiFrame();

        // After ending the ImGui frame, we render the ImGui frame onto the current frame
        // Jonathan Richard 2024-02-10
        renderImGuiFrame();

        glfwSwapBuffers(window);
        glfwPollEvents();
        windowShouldClose = glfwWindowShouldClose(window);
    }
}

application::~application()
{
    shutdownImGui();
    gameEngine.shutdown();
    glfwTerminate();
}

void application::onWindowResize(int width_, int height_)
{
    // Here we update the game engine's display size
    // Similarly we will be able to feed input events into our engine.
    // Jonathan Richard 2024-02-10
    this->width = width_;
    this->height = height_;
    gameEngine.updateDisplay(width_, height_);
}

void application::initImGui()
{
    // Note that we are still using ImGui's GLFW implementation because
    // it's not really of our interest to implement our own GLFW implementation
    // as it doesn't really touch any part of the actual rendering code.
    // We might as well just let ImGui handle it.
    // It basically feeds all the needed input to the ImGui context without us having to do anything.
    // Jonathan Richard 2024-02-10
    ImGui_ImplGlfw_InitForOpenGL(window, true);

    glfwGetWindowSize(window, &width, &height);

    // Initialize the ImGui context with the graphics device interface of engine's renderer
    imguiInstance.initialize(gameEngine.getRenderer().getDevice(), width, height);
}

void application::beginImGuiFrame()
{
    ImGui_ImplGlfw_NewFrame();
    imguiInstance.beginFrame();
}

void application::endImGuiFrame()
{
    imguiInstance.endFrame();
}
void application::renderImGuiFrame()
{
    // Here we render the imgui frame onto the current frame drawn by the engine.
    // We can do this by creating a command buffer and a render pass that will render the imgui frame onto the current frame
    // We could also use the same command buffer that we use to render the game frame by having a callback that is called after the game frame is rendered,
    // passing the command buffer to the imgui renderer and then rendering the imgui frame onto the current frame.
    // But for now since we don't really have an established higher level rendering pipeline,
    // we will create a new command buffer and render pass for the imgui frame.
    //
    // I'm keeping this rendering code here to demonstrate how we can have a decoupled rendering pipeline with a single render pass and thus command buffer
    // If we wanted to we could move this code to ImGuiInstance.h/cpp if we don't want to have to deal with it here.
    // Jonathan Richard 2024-02-10

    auto commandPool = gameEngine.getRenderer().getDevice().createCommandPool({});
    auto commandBuffer = commandPool->acquireCommandBuffer({});

    const RenderPassBeginDesc renderPassDesc = {
            .renderPass = {
                    .colorAttachments = {
                            RenderPassDesc::ColorAttachmentDesc{
                                    LoadAction::Load, // As describe above, don't clear because we are rendering imgui as overlay onto the previous frame
                                    StoreAction::DontCare
                            }
                    }
            },
            // Currently opengl's default framebuffer is implemented as just using nullptr in the render pass
            // However, we could have a framebuffer object that would represent the default framebuffer and we could use it here
            // Jonathan Richard 2024-02-10
            .framebuffer = nullptr
    };

    // Execute the imgui rendering commands
    commandBuffer->beginRenderPass(renderPassDesc);
    std::cout << "Beginning ImGui renderPass" << std::endl;

    imguiInstance.renderFrame(gameEngine.getRenderer().getDevice(), *commandBuffer, nullptr, width, height);
    std::cout << "Ending ImGui renderPass" << std::endl;
    commandBuffer->endRenderPass();
    commandPool->submitCommandBuffer(std::move(commandBuffer));
}

void application::shutdownImGui()
{
    // Cleanup the ImGui context when the application is shutting down
    ImGui_ImplGlfw_Shutdown();
    imguiInstance.shutdown();
}
