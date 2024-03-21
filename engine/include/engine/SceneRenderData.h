//
// Created by Jonathan Richard on 2024-03-04.
//

#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "engine/graphics/Material.h"

class MeshResource;
class MaterialResource;

struct Mesh;

struct MeshRenderData
{
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    std::shared_ptr<MeshResource> mesh = nullptr;
    std::shared_ptr<MaterialResource> material = nullptr;
};

struct LineRenderData
{
    std::vector<glm::vec3> points;
    glm::vec4 color;
};

struct SceneRenderData
{
    std::vector<LineRenderData> lineRenderData;
    std::vector<MeshRenderData> meshRenderData;

public:
    void reset()
    {
        lineRenderData.clear();
        meshRenderData.clear();
    }
};
