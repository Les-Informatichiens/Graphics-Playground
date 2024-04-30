//
// Created by Jonathan Richard on 2024-02-09.
//

#pragma once

#include "Camera.h"
#include "Input.h"
#include "Mesh.h"
#include "MeshRenderer.h"
#include "Model.h"
#include "ResourceManager.h"
#include "SceneRenderer.h"
#include "Stage.h"
#include "TestRenderPass.h"
#include "engine/graphics/Renderer.h"

struct InstanceDesc
{
    std::string assetPath;
    int width;
    int height;
};

class EngineInstance
{
    friend class application;
    friend class SceneEditor;
public:
    explicit EngineInstance(InstanceDesc desc);
    ~EngineInstance();

    void updateDisplay(int width, int height);

    void initialize();
    void updateSimulation(float dt);
    void renderFrame();
    void shutdown();

    graphics::Renderer& getRenderer();
    Stage& getStage();
    Input& getInput();

private:
    Input input;

    graphics::Renderer renderer;

    ResourceManager resourceManager;

    TestRenderPass testRenderPass;

    InstanceDesc desc;

    std::shared_ptr<Camera> activeCamera;

    Stage stage;
    std::shared_ptr<Scene> defaultScene;

    std::shared_ptr<ITexture> testRenderTexture;

    SceneRenderer sceneRenderer;

//    std::shared_ptr<graphics::Material> normalMaterial;
//    std::shared_ptr<graphics::Material> testMaterial;
//    std::shared_ptr<graphics::Material> floorMaterial;
//    std::shared_ptr<graphics::Material> portalMaterial;

    std::shared_ptr<IComputePipeline> computePipeline;

    struct Settings {
        int blockSize;
        int resultWidth;
        int resultHeight;
    };

    std::shared_ptr<IBuffer> computeSettingsBuffer;


    uint8_t calculateMipmapLevels(uint32_t m_width, uint32_t m_height)
    {
        uint32_t width      = m_width  / 2;
        uint32_t height     = m_height / 2;
        uint8_t  mip_levels = 1;

        printf("Mip level %d: %d x %d\n", 0, m_width, m_height);
        printf("Mip level %d: %d x %d\n", mip_levels, width, height);

        for (uint8_t i = 0; i < m_max_iterations; ++i)
        {
            width  = width  / 2;
            height = height / 2;

            if (width < m_downscale_limit || height < m_downscale_limit) break;

            ++mip_levels;

            printf("Mip level %d: %d x %d\n", mip_levels, width, height);
        }

        return mip_levels + 1;
    }

    const uint8_t m_downscale_limit = 10;
    const uint8_t m_max_iterations = 16; // max mipmap levels
    uint8_t bloomMipLevels = 1;

    // Bloom
    struct BloomDownscaleSettings {
        glm::vec4 threshold = glm::vec4(0.0f);
        glm::vec2 texelSize = glm::vec2(0.0f);
        int mipLevel = 0;
        int useThreshold = false;
    };

    struct BloomUpscaleSettings {
        glm::vec2 texelSize = glm::vec2(0.0f);
        int mipLevel = 0;
        float bloomIntensity = 1.0f;
        float dirtIntensity = 0.0f;
    };

    std::array<std::shared_ptr<ITexture>, 2> hdrColorBuffers;
    std::shared_ptr<ISamplerState> hdrSampler;

    std::shared_ptr<ITexture> hdrDepthBuffer;
    std::shared_ptr<IFramebuffer> hdrFramebuffer = nullptr;
    std::shared_ptr<IComputePipeline> bloomDownscalePipeline;
    std::shared_ptr<IBuffer> bloomDownscaleSettingsBuffer;
    std::shared_ptr<IComputePipeline> bloomUpscalePipeline;
    std::shared_ptr<IBuffer> bloomUpscaleSettingsBuffer;

    std::shared_ptr<graphics::VertexData> screenQuadBuffer;

    float bloomThreshold = 1.5f;
    float bloomKnee = 0.1f;
    float bloomIntensity = 1.0f;

    struct PostProcessSettings {
        float exposure = 1.0f;
        float gamma = 2.2f;
        int useFXAA = true;
    } postProcessSettings;

    float pitch = 0.0f;
    float yaw = 0.0f;
    float roll = 0.0f;
};