//
// Created by jeang on 2024-01-25.
//

#include "application.h"
#include "backends/imgui_impl_glfw.h"
#include <iostream>

#include "ImGuiFileDialog.h"

#define STB_IMAGE_IMPLEMENTATION
#include "engine/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "engine/stb_image_write.h"


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

        ImGui::SetNextWindowSizeConstraints(ImVec2(1250, 650), ImVec2(FLT_MAX, FLT_MAX));
        ImGui::Begin("Image Window", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        // Beginning of column layout
        ImGui::Columns(2, "MyColumns", false);
        ImGui::SetColumnWidth(0, 550);


        if (ImGui::BeginTabBar("Tabs", ImGuiTabBarFlags_None))
        {
            // First tab: Import and Export
            if (ImGui::BeginTabItem("Import/Export"))
            {

                ImGui::Text("Image Import:");
                if (ImGui::Button("Import Image"))
                {
                    // Show file dialog to select an image
                    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose an image", ".png,.jpg,.bmp");
                }

                // Check if a file has been selected in the file dialog
                if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
                {
                    if (ImGuiFileDialog::Instance()->IsOk())
                    {
                        // Get the path of the selected image
                        selectedImagePath = ImGuiFileDialog::Instance()->GetFilePathName();

                        // Close the file dialog
                        ImGuiFileDialog::Instance()->Close();
                    }
                }

                // Display the selected image in the "Image Window"
                if (!selectedImagePath.empty())
                {
                    ImGui::InputText("File path", const_cast<char*>(selectedImagePath.c_str()) + 2, selectedImagePath.size(), ImGuiInputTextFlags_None);

                    imageData.pixels = stbi_load(selectedImagePath.c_str(), &imageData.w, &imageData.h, &imageData.comp, STBI_rgb_alpha);
                    auto texDesc = TextureDesc::new2D(TextureFormat::RGBA_UNorm8, imageData.w, imageData.h, TextureDesc::TextureUsageBits::Attachment | TextureDesc::TextureUsageBits::Sampled);
                    imageTexture = gameEngine.getRenderer().getDevice().createTexture(texDesc);
                    imageTexture->upload(imageData.pixels, TextureRangeDesc::new2D(0, 0, imageData.w, imageData.h));
                }


                ImGui::Text("Image Export:");
                if (ImGui::Button("Export Current Image"))
                {
                }

                ImGui::EndTabItem();
            }

            // Second tab: Color Spaces
            if (ImGui::BeginTabItem("Color Spaces"))
            {
                static ImVec4 color = ImVec4(114.0f / 255.0f, 144.0f / 255.0f, 154.0f / 255.0f, 200.0f / 255.0f);
                static bool alpha_preview = true;
                static bool alpha_half_preview = false;
                static bool drag_and_drop = true;
                static bool options_menu = true;
                static bool hdr = false;
                ImGuiColorEditFlags misc_flags = (hdr ? ImGuiColorEditFlags_HDR : 0) | (drag_and_drop ? 0 : ImGuiColorEditFlags_NoDragDrop) | (alpha_half_preview ? ImGuiColorEditFlags_AlphaPreviewHalf : (alpha_preview ? ImGuiColorEditFlags_AlphaPreview : 0)) | (options_menu ? 0 : ImGuiColorEditFlags_NoOptions);

                ImGui::SeparatorText("Color picker");
                static bool alpha = true;
                static bool alpha_bar = true;
                static bool side_preview = true;
                static bool ref_color = false;
                static ImVec4 ref_color_v(1.0f, 0.0f, 1.0f, 0.5f);
                static int display_mode = 0;
                static int picker_mode = 0;
                ImGui::Checkbox("With Alpha", &alpha);
                ImGui::Checkbox("With Alpha Bar", &alpha_bar);
                ImGui::Checkbox("With Side Preview", &side_preview);
                if (side_preview)
                {
                    ImGui::SameLine();
                    ImGui::Checkbox("With Ref Color", &ref_color);
                    if (ref_color)
                    {
                        ImGui::SameLine();
                        ImGui::ColorEdit4("##RefColor", &ref_color_v.x, ImGuiColorEditFlags_NoInputs | misc_flags);
                    }
                }
                ImGui::Combo("Display Mode", &display_mode, "Auto/Current\0None\0RGB Only\0HSV Only\0Hex Only\0");

                ImGui::Text("Set defaults in code:");
                if (ImGui::Button("Default: Uint8 + HSV + Hue Bar"))
                    ImGui::SetColorEditOptions(ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_DisplayHSV | ImGuiColorEditFlags_PickerHueBar);
                if (ImGui::Button("Default: Float + HDR + Hue Wheel"))
                    ImGui::SetColorEditOptions(ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_PickerHueWheel);

                ImGuiColorEditFlags flags = misc_flags;
                if (!alpha) flags |= ImGuiColorEditFlags_NoAlpha;// This is by default if you call ColorPicker3() instead of ColorPicker4()
                if (alpha_bar) flags |= ImGuiColorEditFlags_AlphaBar;
                if (!side_preview) flags |= ImGuiColorEditFlags_NoSidePreview;
                if (picker_mode == 1) flags |= ImGuiColorEditFlags_PickerHueBar;
                if (picker_mode == 2) flags |= ImGuiColorEditFlags_PickerHueWheel;
                if (display_mode == 1) flags |= ImGuiColorEditFlags_NoInputs;  // Disable all RGB/HSV/Hex displays
                if (display_mode == 2) flags |= ImGuiColorEditFlags_DisplayRGB;// Override display mode
                if (display_mode == 3) flags |= ImGuiColorEditFlags_DisplayHSV;
                if (display_mode == 4) flags |= ImGuiColorEditFlags_DisplayHex;

                ImGui::SetNextWindowSizeConstraints(ImVec2(100, 100), ImVec2(300, 300));
                ImGui::ColorPicker4("MyColor##4", (float*) &color, flags, ref_color ? &ref_color_v.x : NULL);

                ImGui::EndTabItem();
            }

            // Third tab: Histogram
            if (ImGui::BeginTabItem("Histogram"))
            {
                ImGui::Text("Histogram:");

                if (ImGui::Button("Calculate and display histogram"))
                {
                }

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::NextColumn();
        ImGui::Separator();
        if (imageTexture)
        {
            ImGui::Text("Image:");
            ImGui::Image((ImTextureID) imageTexture.get(), ImVec2(imageTexture->getWidth(), imageTexture->getHeight()));
        }
        else
        {
            ImGui::Text("No image is loaded.");
        }

        // End of column layout
        ImGui::Columns(1);

        ImGui::End();// End of ImGui window


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
    //    std::cout << "Beginning ImGui renderPass" << std::endl;

    imguiInstance.renderFrame(gameEngine.getRenderer().getDevice(), *commandBuffer, nullptr, width, height);
    //    std::cout << "Ending ImGui renderPass" << std::endl;
    commandBuffer->endRenderPass();
    commandPool->submitCommandBuffer(std::move(commandBuffer));
}

void application::shutdownImGui()
{
    // Cleanup the ImGui context when the application is shutting down
    ImGui_ImplGlfw_Shutdown();
    imguiInstance.shutdown();
}
