//
// Created by Jonathan Richard on 2024-03-04.
//

#pragma once


#include "SceneRenderData.h"
#include "engine/graphics/Renderer.h"

struct SceneCameraDesc
{
    glm::mat4 view;
    glm::mat4 projection;
};

class SceneRenderer
{
public:
    SceneRenderer() = default;
    ~SceneRenderer() = default;

    void render(graphics::Renderer& renderer, const SceneRenderData& sceneData, const SceneCameraDesc& cameraDesc);
};
