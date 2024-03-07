//
// Created by Jonathan Richard on 2024-02-12.
//

#pragma once

#include "Camera.h"
#include "Mesh.h"
#include "Transform.h"
#include "engine/graphics/Renderer.h"

class MeshRenderer
{
public:
    MeshRenderer() = default;

    void render(graphics::Renderer& renderer, const Mesh& mesh, const std::shared_ptr<graphics::Material>&, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection);

private:
    std::shared_ptr<graphics::Renderable> meshRenderable;

};