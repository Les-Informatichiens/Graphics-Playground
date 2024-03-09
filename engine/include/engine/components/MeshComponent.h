//
// Created by Jonathan Richard on 2024-03-05.
//

#pragma once

#include "engine/Mesh.h"
#include "engine/graphics/Material.h"
#include <memory>

class MeshComponent
{
public:
    explicit MeshComponent(std::shared_ptr<Mesh> mesh, std::shared_ptr<graphics::Material> material);
    ~MeshComponent();

    [[nodiscard]] const std::shared_ptr<Mesh>& getMesh() const;
    void setMesh(std::shared_ptr<Mesh> mesh_);

    [[nodiscard]] const std::shared_ptr<graphics::Material>& getMaterial() const;
    void setMaterial(std::shared_ptr<graphics::Material> material_);

private:
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<graphics::Material> material;
};
