//
// Created by Jonathan Richard on 2024-03-04.
//

#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Mesh;

struct MeshRenderData
{
    glm::mat4 modelMatrix;
    Mesh* mesh;
};

struct SceneRenderData
{
    std::vector<MeshRenderData> meshRenderData;

public:
    void reset()
    {
        meshRenderData.clear();
    }
};
