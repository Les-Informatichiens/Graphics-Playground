//
// Created by Jonathan Richard on 2024-03-05.
//

#include <utility>

#include "engine/components/MeshComponent.h"

MeshComponent::MeshComponent(std::shared_ptr<Mesh> mesh)
    : mesh(std::move(mesh))
{
}

MeshComponent::~MeshComponent() = default;

const std::shared_ptr<Mesh>& MeshComponent::getMesh() const
{
    return mesh;
}

void MeshComponent::setMesh(std::shared_ptr<Mesh> mesh_)
{
    this->mesh = std::move(mesh_);
}
