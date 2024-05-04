//
// Created by Jonathan Richard on 2024-02-09.
//

#include "ImGuiInstance.h"

#include <imgui_internal.h>
#include <utility>

namespace imgui {


ImGuiInstance::ImGuiInstance(ImGuiInstanceDesc desc)
{
    context = ImGui::CreateContext();
    context->IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    context->IO.ConfigWindowsMoveFromTitleBarOnly = true;
    ImGui::SetCurrentContext(context);
}

ImGuiInstance::~ImGuiInstance()
{
}

void ImGuiInstance::initialize(IDevice& device, uint32_t width, uint32_t height)
{
    ImGui::SetCurrentContext(context);

    ImGui::StyleColorsDark();

    renderer.initialize(device, width, height);
}

void ImGuiInstance::beginFrame()
{
    ImGui::SetCurrentContext(context);
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
}

void ImGuiInstance::endFrame()
{
    ImGui::SetCurrentContext(context);

    ImGui::EndFrame();
}

void ImGuiInstance::renderFrame(IDevice& device, IGraphicsCommandBuffer& commandBuffer, std::shared_ptr<IGraphicsPipeline> pipeline, uint32_t fbWidth, uint32_t fbHeight)
{
    ImGui::SetCurrentContext(context);
    ImGui::Render();
//    renderer.setupRenderState(ImGui::GetDrawData(), pipeline, *commandBuffer, fbWidth, fbHeight);
    renderer.renderDrawData(ImGui::GetDrawData(), device, commandBuffer, std::move(pipeline));
}

void ImGuiInstance::shutdown()
{
    ImGui::SetCurrentContext(context);

    ImGui::DestroyContext(context);
}

}