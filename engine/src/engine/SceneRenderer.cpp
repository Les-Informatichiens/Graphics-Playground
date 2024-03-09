//
// Created by Jonathan Richard on 2024-03-04.
//

#include "engine/SceneRenderer.h"
#include "engine/MeshRenderer.h"


void SceneRenderer::render(graphics::Renderer& renderer, const SceneRenderData& sceneData, const SceneCameraDesc& cameraDesc)
{
    MeshRenderer meshRenderer;
    for (const auto& meshRenderData: sceneData.meshRenderData)
    {
        meshRenderer.render(renderer, *meshRenderData.mesh, meshRenderData.material, meshRenderData.modelMatrix, cameraDesc.view, cameraDesc.projection);
    }
}
