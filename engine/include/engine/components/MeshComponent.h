//
// Created by Jonathan Richard on 2024-03-05.
//

#pragma once

#include "engine/Mesh.h"
#include "engine/graphics/Material.h"
#include "engine/graphics/MeshResource.h"
#include <memory>

class MeshComponent
{
public:
    explicit MeshComponent(std::shared_ptr<MeshResource> mesh, std::shared_ptr<MaterialResource> material);
    ~MeshComponent();

    [[nodiscard]] std::shared_ptr<MeshResource> getMesh() const;
    [[nodiscard]] std::shared_ptr<MaterialResource> getMaterial() const;

    void setMesh(std::shared_ptr<MeshResource> mesh);
    void setMaterial(std::shared_ptr<MaterialResource> material);

private:
    std::shared_ptr<MeshResource> mesh;
    std::shared_ptr<MaterialResource> material;
};
