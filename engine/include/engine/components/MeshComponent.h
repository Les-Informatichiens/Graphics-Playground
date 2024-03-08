//
// Created by Jonathan Richard on 2024-03-05.
//

#pragma once

#include "engine/Mesh.h"
#include <memory>

class MeshComponent
{
public:
    explicit MeshComponent(std::shared_ptr<Mesh> mesh);
    ~MeshComponent();

    [[nodiscard]] const std::shared_ptr<Mesh>& getMesh() const;
    void setMesh(std::shared_ptr<Mesh> mesh_);

private:
    std::shared_ptr<Mesh> mesh;
};
