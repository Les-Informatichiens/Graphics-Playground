//
// Created by Jonathan Richard on 2024-03-04.
//

#pragma once

#include "Light.h"
#include "engine/graphics/Material.h"
#include <glm/glm.hpp>
#include <vector>

class MeshResource;
class MaterialResource;
class TextureResource;
class Light;

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

struct LightData
{
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::vec3 position;
    glm::vec3 direction;
    const Light* light;
};

struct SkyboxData
{
    std::shared_ptr<TextureResource> texture;
};

struct SceneRenderData
{
    std::vector<LineRenderData> lineRenderData;
    std::vector<MeshRenderData> meshRenderData;
    std::vector<LightData> lights;
    SkyboxData skybox;
    int lightModel = 0; // test variable

public:
    void reset()
    {
        lineRenderData.clear();
        meshRenderData.clear();
        lights.clear();
        skybox.texture = nullptr;
    }
};
