//
// Created by jeang on 2024-01-25.
//

#include "backends/imgui_impl_glfw.h"
#include "engine/components/CameraComponent.h"
#include <cstring>

#include "ImGuiFileDialog.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG//generate user friendly error messages
#include "engine/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "engine/stb_image_write.h"

#include "application.h"

RayTracer rayTracerz;


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
        auto* app = (application*) glfwGetWindowUserPointer(window);
        app->onKey(key, scancode, action, mods);
    });
    glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
        auto* app = (application*) glfwGetWindowUserPointer(window);
        app->onMouseButton(button, action, mods);
    });
    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
        auto* app = (application*) glfwGetWindowUserPointer(window);
        app->onMouseMove(xpos, ypos);
    });

    glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
        auto* app = (application*) glfwGetWindowUserPointer(window);
        app->onMouseScroll(xoffset, yoffset);
    });

    // add window resize callback and bind this object's pointer to it
    glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width_, int height_) {
        auto* app = (application*) glfwGetWindowUserPointer(window);
        app->onWindowResize(width_, height_);
    });

    glfwMakeContextCurrent(window);

    // The game engine needs to be initialized before the ImGui context,
    // because the ImGui context needs the game engine's renderer to be initialized
    // Jonathan Richard 2024-02-10
    gameEngine.initialize();

    // Initialize the ImGui context
    initImGui();
    vectorDrawer = picasso();
    RTimageData.comp = STBI_rgb;
    RTimageData.w = rayTracerz.rgb_image.get_width();
    RTimageData.h = rayTracerz.rgb_image.get_height();
    auto texDesc = TextureDesc::new2D(TextureFormat::RGBX_UNorm8, RTimageData.w, RTimageData.h, TextureDesc::TextureUsageBits::Attachment | TextureDesc::TextureUsageBits::Sampled);
    RTtexture = gameEngine.getRenderer().getDevice().createTexture(texDesc);
}

std::unique_ptr<unsigned char[]> originalImageData;


void calculateRGBHistogram(unsigned char* pixels, int width, int height, float* histogram)
{
    int numBins = 256;              // 256 bins for each RGB channel
    int histogramSize = 3 * numBins;// 3 channels (RGB)
    std::fill(histogram, histogram + histogramSize, 0.0f);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int pixelIndex = (y * width + x) * 4;
            int r = pixels[pixelIndex];
            int g = pixels[pixelIndex + 1];
            int b = pixels[pixelIndex + 2];

            histogram[r]++;
            histogram[numBins + g]++;
            histogram[2 * numBins + b]++;
        }
    }

    const float totalPixels = height * width;
    for (int i = 0; i < histogramSize; ++i)
    {
        histogram[i] /= totalPixels;
    }
}

void application::run()
{

    while (!windowShouldClose)
    {
        //update engine input with glfw input
        auto& input = gameEngine.getInput();

        // We will update the simulation and render the frame before rendering the ImGui frame
        // This is to simulate the game engine running and rendering a frame before the ImGui frame is rendered on top of it
        // Jonathan Richard 2024-02-10
        gameEngine.updateSimulation(0.0f);

        //        if (auto scene = gameEngine.getStage().getScene())
        //        {
        //
        //            auto viewer = scene->getEntityByName("viewer");
        //            if (viewer)
        //            {
        //                auto& viewerNode = viewer->getSceneNode();
        //                // make it turn in circles with system clock
        //                if (cameraMotion)
        //                    viewerNode.getTransform().setPosition({20.0f * glm::cos((float)clock()/1000.0f), 15.0f, 20.0f * glm::sin((float)clock()/1000.0f)});
        //
        //                if (lockCamOnSelected)
        //                {
        //                    auto selected = sceneEditor.getLastSelectedEntity();
        //                    if (selected.first && scene->getEntity(selected.second)->getName() != "viewer")
        //                    {
        //                        auto& selectedNode = scene->getEntity(selected.second)->getSceneNode();
        //                        viewerNode.getTransform().lookAt(selectedNode.getWorldTransform().getPosition(), glm::vec3(0.0f, 1.0f, 0.0f));
        //                    }
        //                    else
        //                    {
        //                        viewerNode.getTransform().lookAt({0.0f, 0.0f, 0.0f}, glm::vec3(0.0f, 1.0f, 0.0f));
        //                    }
        //                }
        //                else
        //                {
        //                    viewerNode.getTransform().lookAt({0.0f, 0.0f, 0.0f}, glm::vec3(0.0f, 1.0f, 0.0f));
        //                }
        //            }
        //        }


        gameEngine.renderFrame();

        beginImGuiFrame();

        if (showCurvesUwu)
        {
            ImGui::SetNextWindowSizeConstraints(ImVec2(1200, 600), ImVec2(FLT_MAX, FLT_MAX));
            ImGui::Begin("Curves", &showCurvesUwu, ImGuiWindowFlags_AlwaysAutoResize);


            static bool showSurface = false;
            static bool showSpline = false;

            constexpr ImU32 color = IM_COL32(255, 215, 0, 120);
            constexpr ImU32 color2 = IM_COL32(255, 0, 0, 255);

            // Sliders Window
            ImGui::BeginChild("Slider Window", ImVec2(350, 0), true);
            {
                // Surface
                ImGui::Checkbox("Show Coon Surface", &showSurface);
                if (showSurface)
                {

                    for (int i = 0; i < 4; ++i)
                    {
                        //ImGui::Text(("Corner " + std::to_string(i + 1)).c_str());
                        //ImGui::SetNextItemWidth(125);
                        ImGui::SliderFloat(("Corner " + std::to_string(i + 1) + " X").c_str(), &corners[i][0], 0.0f, 1.0f);
                        //ImGui::SetNextItemWidth(125);
                        ImGui::SliderFloat(("Corner " + std::to_string(i + 1) + " Y").c_str(), &corners[i][1], 0.0f, 1.0f);
                        ImGui::NewLine();
                    }

                    ImGui::NewLine(); ImGui::NewLine();

                    ImGui::SliderFloat("Amplitude X", &Ax, 0.0f, 100.0f);
                    //ImGui::SameLine();
                    ImGui::SliderFloat("Amplitude Y", &Ay, 0.0f, 100.0f);

                    ImGui::NewLine();

                    ImGui::SliderFloat("Fréquence X", &fx, 0.0f, 10.0f);
                    //ImGui::SameLine();
                    ImGui::SliderFloat("Fréquence Y", &fy, 0.0f, 10.0f);

                }

                // Spline
                ImGui::Checkbox("Show Spline", &showSpline);
                if (showSpline)
                {
                    for (int i = 0; i < 5; ++i)
                    {
                        ImGui::SliderFloat(("Control Point " + std::to_string(i + 1) + " X").c_str(), &controlPoints[i][0], 0.0f, 1.0f);
                        ImGui::SliderFloat(("Control Point " + std::to_string(i + 1) + " Y").c_str(), &controlPoints[i][1], 0.0f, 1.0f);
                        ImGui::NewLine();
                    }

                }

                ImGui::EndChild();
            }

            ImGui::SameLine();

            // Drawing Window
            ImGui::BeginChild("Drawing Window", ImVec2(0, 0), true);
            {
                ImDrawList* draw_list = ImGui::GetWindowDrawList();

                if (showSurface)
                {
                    curvesDrawer.DrawCoonsSurface(draw_list, corners, color, Ax, Ay, fx, fy);
                }

                if (showSpline)
                {
                    CurvesDrawer::DrawBezierSpline(draw_list, controlPoints, color2, 2.0f);
                }

                ImGui::EndChild();
            }

            ImGui::End();
        }

        if (showRayTracer)
        {
            ImGui::SetNextWindowSizeConstraints(ImVec2(1250, 650), ImVec2(FLT_MAX, FLT_MAX));
            ImGui::Begin("Ray Tracer", &showRayTracer, ImGuiWindowFlags_AlwaysAutoResize);
            if (!renderedImage)
            {
                renderedImage = rayTracerz.run();
                RTimageData.pixels = rayTracerz.rgb_image.get_pixel_data();
            }

            if (RTtexture)
            {
                RTtexture->upload(RTimageData.pixels, TextureRangeDesc::new2D(0, 0, RTimageData.w, RTimageData.h));
                ImGui::Text("RTImage:");
                ImGui::Image(RTtexture.get(), ImVec2(RTtexture->getWidth(), RTtexture->getHeight()));
            }
            ImGui::End();
        }
        else
        {
            renderedImage = false;
        }


        // raycasting
        {
            // if right click, raycastselection
            if (input.isMouseButtonPressed(0))
            {
                raycastSelection();
            }
        }

        if (showImageWindow)
        {

            ImGui::SetNextWindowSizeConstraints(ImVec2(1250, 650), ImVec2(FLT_MAX, FLT_MAX));
            ImGui::Begin("Image", &showImageWindow, ImGuiWindowFlags_AlwaysAutoResize);

            // Beginning of column layout
            ImGui::Columns(2, "MyColumns", false);
            ImGui::SetColumnWidth(0, 550);


            if (ImGui::BeginTabBar("Tabs", ImGuiTabBarFlags_None))
            {
                // First tab: Import and Export
                if (ImGui::BeginTabItem("Import"))
                {

                    ImGui::Text("Image Import:");
                    if (ImGui::Button("Import Image"))
                    {
                        // Show file dialog to select an image
                        ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose an image", ".png,.jpg");
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

                        imageData.comp = STBI_rgb_alpha;
                        imageData.pixels = stbi_load(selectedImagePath.c_str(), &imageData.w, &imageData.h, &imageData.originalComp, STBI_rgb_alpha);
                        auto texDesc = TextureDesc::new2D(TextureFormat::RGBA_UNorm8, imageData.w, imageData.h, TextureDesc::TextureUsageBits::Attachment | TextureDesc::TextureUsageBits::Sampled);
                        imageTexture = gameEngine.getRenderer().getDevice().createTexture(texDesc);
                        imageTexture->upload(imageData.pixels, TextureRangeDesc::new2D(0, 0, imageData.w, imageData.h));
                        originalImageData.reset(new unsigned char[imageData.w * imageData.h * imageData.comp]);
                        std::memcpy(originalImageData.get(), imageData.pixels, imageData.w * imageData.h * imageData.comp);
                    }

                    for (int i = 0; i < 5; ++i)
                    {
                        ImGui::Spacing();
                    }

                    ImGui::EndTabItem();
                }

                // Second tab: Color Spaces
                if (ImGui::BeginTabItem("Image Editor"))
                {
                    static ImVec4 color = ImVec4(0.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f, 128.0f / 255.0f);

                    // COLOR FILTER
                    ImGuiColorEditFlags flags = ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_DisplayHSV | ImGuiColorEditFlags_DisplayHex;

                    ImGui::SetNextWindowSizeConstraints(ImVec2(100, 100), ImVec2(300, 300));
                    ImGui::Text("Color filter :");
                    ImGui::ColorPicker4("MyColor##4", (float*) &color, flags, NULL);

                    if (ImGui::Button("Apply filter"))
                    {

                        if (imageData.pixels != nullptr && originalImageData != nullptr)
                        {
                            std::memcpy(imageData.pixels, originalImageData.get(), imageData.w * imageData.h * imageData.comp);
                            // Create a new texture to hold the color layer
                            std::unique_ptr<unsigned char[]> colorLayer(new unsigned char[imageData.w * imageData.h * imageData.comp]);

                            // Fill the color layer with the selected color
                            for (int i = 0; i < imageData.w * imageData.h * imageData.comp; i += imageData.comp)
                            {
                                colorLayer[i] = static_cast<unsigned char>(color.x * 255.0f);
                                colorLayer[i + 1] = static_cast<unsigned char>(color.y * 255.0f);
                                colorLayer[i + 2] = static_cast<unsigned char>(color.z * 255.0f);
                                colorLayer[i + 3] = static_cast<unsigned char>(color.w * 255.0f);// 50% alpha
                            }

                            // Mix the color layer with the original image
                            for (int y = 0; y < imageData.h; ++y)
                            {
                                for (int x = 0; x < imageData.w; ++x)
                                {
                                    int pixelIndex = (y * imageData.w + x) * imageData.comp;

                                    imageData.pixels[pixelIndex] = static_cast<unsigned char>((imageData.pixels[pixelIndex] * (255 - colorLayer[pixelIndex + 3]) + colorLayer[pixelIndex] * colorLayer[pixelIndex + 3]) / 255);
                                    imageData.pixels[pixelIndex + 1] = static_cast<unsigned char>((imageData.pixels[pixelIndex + 1] * (255 - colorLayer[pixelIndex + 3]) + colorLayer[pixelIndex + 1] * colorLayer[pixelIndex + 3]) / 255);
                                    imageData.pixels[pixelIndex + 2] = static_cast<unsigned char>((imageData.pixels[pixelIndex + 2] * (255 - colorLayer[pixelIndex + 3]) + colorLayer[pixelIndex + 2] * colorLayer[pixelIndex + 3]) / 255);
                                }
                            }

                            // Upload the modified image data to the texture
                            imageTexture->upload(imageData.pixels, TextureRangeDesc::new2D(0, 0, imageData.w, imageData.h));
                        }
                    }

                    for (int i = 0; i < 5; ++i)
                    {
                        ImGui::Spacing();
                    }

                    // INVERT COLORS
                    ImGui::Text("Invert colors :");

                    if (ImGui::Button("Invert Colors"))
                    {
                        if (imageData.pixels != nullptr)
                        {
                            for (int y = 0; y < imageData.h; ++y)
                            {
                                for (int x = 0; x < imageData.w; ++x)
                                {
                                    int pixelIndex = (y * imageData.w + x) * imageData.comp;

                                    imageData.pixels[pixelIndex] = 255 - imageData.pixels[pixelIndex];        // Rouge
                                    imageData.pixels[pixelIndex + 1] = 255 - imageData.pixels[pixelIndex + 1];// Vert
                                    imageData.pixels[pixelIndex + 2] = 255 - imageData.pixels[pixelIndex + 2];// Bleu

                                    imageData.pixels[pixelIndex] = std::clamp(imageData.pixels[pixelIndex], uint8_t(0), uint8_t(255));
                                    imageData.pixels[pixelIndex + 1] = std::clamp(imageData.pixels[pixelIndex + 1], uint8_t(0), uint8_t(255));
                                    imageData.pixels[pixelIndex + 2] = std::clamp(imageData.pixels[pixelIndex + 2], uint8_t(0), uint8_t(255));
                                }
                            }

                            imageTexture->upload(imageData.pixels, TextureRangeDesc::new2D(0, 0, imageData.w, imageData.h));
                        }
                    }

                    for (int i = 0; i < 5; ++i)
                    {
                        ImGui::Spacing();
                    }

                    //EXPORT
                    ImGui::Text("Image Export:");
                    if (ImGui::Button("Export Current Image"))
                    {
                        if (imageData.pixels != nullptr)
                        {
                            ImGuiFileDialog::Instance()->OpenDialog("ExportFileDialogKey", "Save Image As", ".png,.jpg");// Permet d'exporter en PNG ou en JPG
                        }
                    }

                    if (ImGuiFileDialog::Instance()->Display("ExportFileDialogKey"))
                    {
                        if (ImGuiFileDialog::Instance()->IsOk())
                        {
                            std::string savePath = ImGuiFileDialog::Instance()->GetFilePathName();
                            ImGuiFileDialog::Instance()->Close();

                            int width = imageData.w;
                            int height = imageData.h;
                            int channels = 4;// RGBA

                            // Check the selected extension
                            std::string extension = savePath.substr(savePath.find_last_of(".") + 1);
                            if (extension == "png")
                            {
                                stbi_write_png(savePath.c_str(), width, height, channels, imageData.pixels, width * channels);
                            }
                            else if (extension == "jpg" || extension == "jpeg")
                            {
                                stbi_write_jpg(savePath.c_str(), width, height, channels, imageData.pixels, 100);
                            }
                        }
                    }

                    ImGui::EndTabItem();
                }

                // Third tab: Histogram
                if (ImGui::BeginTabItem("Histogram"))
                {

                    if (imageData.pixels != nullptr)
                    {
                        int numBins = 256;
                        int histogramSize = 3 * numBins;// 3 channels (RGB)
                        float* histogram = new float[histogramSize];

                        calculateRGBHistogram(imageData.pixels, imageData.w, imageData.h, histogram);

                        // draw colored bands from imgui primitives
                        ImGui::Text("Histogram:");
                        ImDrawList* draw_list = ImGui::GetWindowDrawList();
                        ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
                        ImVec2 canvas_size = ImVec2(256, 100);
                        float histogramMax = 0.0f;
                        for (int i = 0; i < histogramSize; ++i)
                        {
                            if (histogram[i] > histogramMax)
                            {
                                histogramMax = histogram[i];
                            }
                        }
                        for (int i = 0; i < histogramSize; ++i)
                        {
                            ImVec4 bandColor;
                            // put each colored section on a new line
                            float value = histogram[i] / histogramMax;

                            int yOffset = 0;
                            if (i < numBins)
                            {
                                bandColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);// Red
                                yOffset = 0;
                                draw_list->AddRectFilled(ImVec2(canvas_pos.x + i, canvas_pos.y + canvas_size.y * (1 - value) - yOffset), ImVec2(canvas_pos.x + i + 1, canvas_pos.y + canvas_size.y - yOffset), IM_COL32(255, 0, 0, 255));
                            }
                            else if (i < 2 * numBins)
                            {
                                bandColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);// Green
                                yOffset = -1 * 100;
                                draw_list->AddRectFilled(ImVec2(canvas_pos.x + i - numBins, canvas_pos.y + canvas_size.y * (1 - value) - yOffset), ImVec2(canvas_pos.x + i - numBins + 1, canvas_pos.y + canvas_size.y - yOffset), IM_COL32(0, 255, 0, 255));
                            }
                            else
                            {
                                bandColor = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);// Blue
                                yOffset = -2 * 100;
                                draw_list->AddRectFilled(ImVec2(canvas_pos.x + i - 2 * numBins, canvas_pos.y + canvas_size.y * (1 - value) - yOffset), ImVec2(canvas_pos.x + i - 2 * numBins + 1, canvas_pos.y + canvas_size.y - yOffset), IM_COL32(0, 0, 255, 255));
                            }
                        }
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
        }


        // Here we can have some ImGui code that would let the user
        // control some state in the application.
        // We can later put this in a better place, like separate classes
        // that would handle the ImGui code for different parts of the application
        // Jonathan Richard 2024-02-10

        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Menu");// Create a window called "Hello, world!" and append into it.

            ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bools storing our window open/close state
            ImGui::Checkbox("Image Manipulation", &showImageWindow);
            ImGui::Checkbox("Ray Tracing", &showRayTracer);
            ImGui::Checkbox("Curves", &showCurvesUwu);
            ImGui::Checkbox("Vector drawing window", &show_another_window);
            ImGui::Checkbox("Scene Editor", &show_editor);
            ImGui::Checkbox("Camera Render Texture", &show_pov_cam);

            ImGui::SeparatorText("Tone mapping");
            ImGui::SliderFloat("Exposure", &gameEngine.postProcessSettings.exposure, 0.0, 10.0, "%.1f");
            ImGui::SliderFloat("Gamma", &gameEngine.postProcessSettings.gamma, 0.0, 10.0, "%.1f");
            ImGui::Checkbox("Apply Tone Mapping", reinterpret_cast<bool*>(&gameEngine.postProcessSettings.toneMap));

            ImGui::SeparatorText("Anti-aliasing");
            ImGui::Checkbox("FXAA", reinterpret_cast<bool*>(&gameEngine.postProcessSettings.useFXAA));

            ImGui::SeparatorText("Lighting");
            ImGui::Combo("Light Model", &gameEngine.lightModel, "PBR\0Blinn-Phong\0Phong\0Gouraud\0Toon\0\0");

            ImGui::SeparatorText("Bloom");
            ImGui::SliderFloat("Bloom threshold",      &gameEngine.bloomThreshold,            0.0f, 15.0f, "%.1f");
            ImGui::SliderFloat("Bloom knee",           &gameEngine.bloomKnee,                 0.0f, 1.0f,  "%.1f");
            ImGui::SliderFloat("Bloom intensity",      &gameEngine.bloomIntensity,      0.0f, 5.0f,  "%.1f");

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            ImGui::End();

            if (show_demo_window)
            {
                ImGui::ShowDemoWindow(&show_demo_window);
            }

            if (show_another_window)
            {
                ImGui::Begin("Vector drawing window", &show_another_window);
                vectorDrawer.draw(ImGui::GetBackgroundDrawList());
                ImGui::End();
            }

            if (show_editor)
            {
                sceneEditor.draw();
            }

            if (show_pov_cam)
            {
                ImGui::Begin("Camera", &show_pov_cam);
                auto cameraEntity = gameEngine.getStage().getScene()->getEntityByName("teapotPOV");
                if (cameraEntity)
                {
                    auto& camera = cameraEntity->getComponent<CameraComponent>();
                    // allow the image to scale according to the window size
                    ImGui::Image((ImTextureID) camera.getRenderTarget().colorTexture.get(), ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y));
                }
                ImGui::End();
            }
        }

        // ImGui UI calls should always be done between begin and end frame
        endImGuiFrame();

        // After ending the ImGui frame, we render the ImGui frame onto the current frame
        // Jonathan Richard 2024-02-10
        renderImGuiFrame();

        input.update();


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
    auto commandBuffer = commandPool->acquireGraphicsCommandBuffer({});

    const RenderPassBeginDesc renderPassDesc = {
            .renderPass = {
                    .colorAttachments = {
                            RenderPassDesc::ColorAttachmentDesc{
                                    LoadAction::Load,// As describe above, don't clear because we are rendering imgui as overlay onto the previous frame
                                    StoreAction::DontCare}}},
            // Currently opengl's default framebuffer is implemented as just using nullptr in the render pass
            // However, we could have a framebuffer object that would represent the default framebuffer and we could use it here
            // Jonathan Richard 2024-02-10
            .framebuffer = nullptr};

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

void application::onKey(int key, int scancode, int action, int mods)
{
    bool pressed = (action == GLFW_PRESS || action == GLFW_REPEAT) && action != GLFW_RELEASE;
    gameEngine.getInput().setKeyPressed(key, pressed);
}

void application::onMouseButton(int button, int action, int mods)
{
    bool pressed = (action == GLFW_PRESS) && action != GLFW_RELEASE;

    // return if ontop of imgui
    if (ImGui::GetIO().WantCaptureMouse)
        return;

    gameEngine.getInput().setMouseButtonPressed(button, pressed);
}

void application::onMouseMove(double xpos, double ypos)
{
    gameEngine.getInput().setMousePosition(xpos, ypos);
}

void application::onMouseScroll(double xoffset, double yoffset)
{
    // return if ontop of imgui
    if (ImGui::GetIO().WantCaptureMouse)
        return;
    gameEngine.getInput().setMouseWheel(yoffset);
}


void application::raycastSelection()
{
    auto scene = gameEngine.getStage().getScene();
    if (!scene)
        return;

    auto cameraEntity = scene->getEntityByName("viewer");//scene->findMainCameraEntity();
    auto cameraNode = cameraEntity->getSceneNode();
    auto camera = cameraEntity->getComponent<CameraComponent>().getCamera();

    float screenX, screenY;
    gameEngine.getInput().getMousePosition(screenX, screenY);

    auto ray = camera->screenPointToRay(cameraNode.getWorldTransform(), screenX, screenY, width, height);
    std::optional<RaycastHit> hit = scene->raycastFirstHit(ray, 100000000.0f);
    if (hit)
    {
        sceneEditor.clearSelection();
        sceneEditor.setEntitySelection(hit->entityUUID, true);
    }
    else
    {
        sceneEditor.clearSelection();
    }

    ImGui::Begin("Raycast Selection");
    ImGui::Text("Ray Origin: (%.2f, %.2f, %.2f)", ray.getOrigin().x, ray.getOrigin().y, ray.getOrigin().z);
    ImGui::Text("Ray Direction: (%.2f, %.2f, %.2f)", ray.getDirection().x, ray.getDirection().y, ray.getDirection().z);
    if (hit)
    {
        ImGui::Text("Hit Point: (%.2f, %.2f, %.2f)", hit->hitPoint.x, hit->hitPoint.y, hit->hitPoint.z);
        ImGui::Text("Hit Normal: (%.2f, %.2f, %.2f)", hit->hitNormal.x, hit->hitNormal.y, hit->hitNormal.z);
    }
    ImGui::End();

    // for debugging, remove after and anything that uses this
    if (hit)
    {
        gameEngine.hitp = hit->hitPoint;
    }
    gameEngine.ray = ray;
}