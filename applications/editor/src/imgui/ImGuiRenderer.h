//
// Created by Jonathan Richard on 2024-02-09.
//

#pragma once

#include "graphicsAPI/common/Device.h"
#include "imgui.h"

namespace imgui {

class ImGuiRenderer
{
public:
    ImGuiRenderer() = default;
    ~ImGuiRenderer() = default;

    void initialize(IDevice& device, uint32_t width, uint32_t height);
    void newFrame();
    void setupRenderState(ImDrawData* drawData, std::shared_ptr<IGraphicsPipeline>& pipeline, IGraphicsCommandBuffer& commandBuffer, uint32_t fbWidth, uint32_t fbHeight);
    void renderDrawData(ImDrawData* drawData, IDevice& device, IGraphicsCommandBuffer& commandBuffer, std::shared_ptr<IGraphicsPipeline> pipeline);

private:
    std::shared_ptr<IBuffer> vertexBuffer;
    std::shared_ptr<IBuffer> indexBuffer;
    std::shared_ptr<IBuffer> uniformBuffer;

    std::shared_ptr<ITexture> fontTexture;
    std::shared_ptr<ISamplerState> samplerState;

    std::shared_ptr<IGraphicsPipeline> pipeline;
    std::shared_ptr<IDepthStencilState> depthStencilState;
    std::unordered_map<ImTextureID, std::shared_ptr<ITexture>> textureMap;

    static constexpr bool isOpenGL = true;
};

}