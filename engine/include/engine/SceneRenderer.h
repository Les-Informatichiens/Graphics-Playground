//
// Created by Jonathan Richard on 2024-03-04.
//

#pragma once


#include "SceneRenderData.h"
#include "engine/graphics/Renderer.h"


class SceneRenderer
{
public:
    SceneRenderer() = default;
    ~SceneRenderer() = default;

    void render(graphics::Renderer& renderer, SceneRenderData& sceneData);
};
