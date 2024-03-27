//
// Created by Jonathan Richard on 2024-03-04.
//

#include "engine/SceneRenderer.h"
#include "engine/LineRenderer.h"
#include "engine/MeshRenderer.h"


void SceneRenderer::render(graphics::Renderer& renderer, const SceneRenderData& sceneData, const SceneCameraDesc& cameraDesc)
{
    MeshRenderer meshRenderer;
    for (const auto& meshRenderData: sceneData.meshRenderData)
    {
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
