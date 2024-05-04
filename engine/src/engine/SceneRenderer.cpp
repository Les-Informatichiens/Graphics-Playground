//
// Created by Jonathan Richard on 2024-03-04.
//

#include "engine/SceneRenderer.h"
#include "engine/LineRenderer.h"
#include "engine/MeshRenderer.h"
#include "engine/graphics/MaterialResource.h"


void SceneRenderer::render(graphics::Renderer& renderer, const SceneRenderData& sceneData, const SceneCameraDesc& cameraDesc)
{
    MeshRenderer meshRenderer;
    for (const auto& meshRenderData: sceneData.meshRenderData)
    {
        struct MVPUBO {
            glm::mat4 model;
            glm::mat4 view;
            glm::mat4 proj;
        } ubo(meshRenderData.modelMatrix, cameraDesc.view, cameraDesc.projection);

        struct Constants {
            alignas(16) glm::vec3 cameraPos;
            alignas(16) glm::vec3 cameraDirection;
            alignas(16) glm::vec3 lightDir;
            int lightModel = 0;
        } constants(cameraDesc.position, cameraDesc.direction, glm::vec3(-0.7f, -0.5f, -0.5f), sceneData.lightModel);

        meshRenderData.material->setUniformBuffer("ubo", &ubo, sizeof(ubo), 0);
        meshRenderData.material->setUniformBuffer("constants", &constants, sizeof(constants), 1);

        // update pbr lights
        {
            auto& pbrMaterial = meshRenderData.material;

            constexpr int MAX_LIGHTS = 10;

            struct LightUniform {
                alignas(16) glm::vec3 position;
                alignas(16) glm::vec3 color;
                float intensity;
                float constant;
                float linear;
                float quadratic;
            };

            struct SpotlightUniform {
                alignas(16) glm::vec3 position;
                alignas(16) glm::vec3 direction;
                alignas(16) glm::vec3 color;

                float cutoff;
                float outerCutoff;

                float intensity;
                float constant;
                float linear;
                float quadratic;
            };

            struct PointLightsUBO
            {
                int numLights;
                LightUniform lights[MAX_LIGHTS];
            };

            struct SpotlightsUBO
            {
                int numLights;
                SpotlightUniform lights[MAX_LIGHTS];
            };

            struct DirecitonalLightUniform
            {
                alignas(16) glm::vec3 color;
                alignas(16) glm::vec3 direction;
                float intensity;
            };

            struct DirectionalLightsUBO
            {
                DirecitonalLightUniform light;
            };

            PointLightsUBO pointLightsUbo{};
            SpotlightsUBO spotlightsUbo{};
            DirectionalLightsUBO directionalLightsUbo{};


            for (int i = 0; i < sceneData.lights.size(); i++)
            {
                auto item = sceneData.lights[i];
                switch (item.light->getType())
                {
                    case LightType::Point:
                    {
                        auto& pointLight = item.light;
                        pointLightsUbo.lights[pointLightsUbo.numLights].position = item.position;
                        pointLightsUbo.lights[pointLightsUbo.numLights].color = pointLight->getColor();
                        pointLightsUbo.lights[pointLightsUbo.numLights].intensity = pointLight->getIntensity();
                        pointLightsUbo.lights[pointLightsUbo.numLights].constant = pointLight->getConstant();
                        pointLightsUbo.lights[pointLightsUbo.numLights].linear = pointLight->getLinear();
                        pointLightsUbo.lights[pointLightsUbo.numLights].quadratic = pointLight->getQuadratic();
                        pointLightsUbo.numLights++;
                        break;
                    }
                    case LightType::Spot:
                    {
                        auto& spotlight = item.light;
                        spotlightsUbo.lights[spotlightsUbo.numLights].position = item.position;
                        spotlightsUbo.lights[spotlightsUbo.numLights].direction = item.direction;
                        spotlightsUbo.lights[spotlightsUbo.numLights].color = spotlight->getColor();
                        spotlightsUbo.lights[spotlightsUbo.numLights].cutoff = glm::cos(glm::radians(spotlight->getCutOff()));
                        spotlightsUbo.lights[spotlightsUbo.numLights].outerCutoff = glm::cos(glm::radians(spotlight->getOuterCutOff()));
                        spotlightsUbo.lights[spotlightsUbo.numLights].intensity = spotlight->getIntensity();
                        spotlightsUbo.lights[spotlightsUbo.numLights].constant = spotlight->getConstant();
                        spotlightsUbo.lights[spotlightsUbo.numLights].linear = spotlight->getLinear();
                        spotlightsUbo.lights[spotlightsUbo.numLights].quadratic = spotlight->getQuadratic();
                        spotlightsUbo.numLights++;
                        break;
                    }
                    case LightType::Directional:
                    {
                        auto& directionalLight = item.light;
                        directionalLightsUbo.light.color = directionalLight->getColor();
                        directionalLightsUbo.light.direction = item.direction;
                        directionalLightsUbo.light.intensity = directionalLight->getIntensity();
                        break;
                    }
                }
            }

            pbrMaterial->getMaterial()->setUniformBytes("Lights", &pointLightsUbo, sizeof(pointLightsUbo), 6); // binding 6
            pbrMaterial->getMaterial()->setUniformBytes("Spotlights", &spotlightsUbo, sizeof(spotlightsUbo), 7); // binding 7
            pbrMaterial->getMaterial()->setUniformBytes("DirectionalLight", &directionalLightsUbo, sizeof(directionalLightsUbo), 8); // binding 8
        }

        meshRenderer.render(renderer, meshRenderData.mesh, meshRenderData.material, meshRenderData.modelMatrix, cameraDesc.view, cameraDesc.projection, cameraDesc.position, cameraDesc.direction);
    }

    LineRenderer lineRenderer;
    lineRenderer.setVP(cameraDesc.view, cameraDesc.projection);

    lineRenderer.setLineWidth(0.005f);
//    lineRenderer.setMiter(1);

    lineRenderer.setAspect((float)cameraDesc.viewportWidth / (float)cameraDesc.viewportHeight);

    for (const auto& lineRenderData: sceneData.lineRenderData)
    {
        lineRenderer.setColor(lineRenderData.color);
        lineRenderer.drawLines(renderer, lineRenderData.points);
    }
}
