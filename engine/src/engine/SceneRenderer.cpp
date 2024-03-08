//
// Created by Jonathan Richard on 2024-03-04.
//

#include "engine/SceneRenderer.h"
#include "engine/MeshRenderer.h"


void SceneRenderer::render(graphics::Renderer& renderer, SceneRenderData& sceneData)
{
    MeshRenderer meshRenderer;
    for (auto& meshRenderData: sceneData.meshRenderData)
    {
        meshRenderer.render(renderer, *meshRenderData.mesh, meshRenderData.modelMatrix);
    }
}
