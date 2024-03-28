//
// Created by Jonathan Richard on 2024-03-04.
//

#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "engine/graphics/Material.h"

struct Mesh;

struct MeshRenderData
{
    glm::mat4 modelMatrix;
    Mesh* mesh;
    std::shared_ptr<graphics::Material> material;
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
        meshRenderData.clear();
    }
};
