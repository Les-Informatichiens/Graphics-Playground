//
// Created by Jonathan Richard on 2024-02-12.
//

#pragma once

#include "Camera.h"
#include "Mesh.h"
#include "Transform.h"
#include "engine/graphics/Renderer.h"

class MaterialResource;
class MeshResource;

class MeshRenderer
{
public:
    MeshRenderer() = default;

    void render(graphics::Renderer& renderer, const std::shared_ptr<MeshResource>& mesh, const std::shared_ptr<MaterialResource>&, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
};