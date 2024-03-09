//
// Created by Jonathan Richard on 2024-03-05.
//

#include <utility>

#include "engine/components/MeshComponent.h"

MeshComponent::MeshComponent(std::shared_ptr<Mesh> mesh, std::shared_ptr<graphics::Material> material)
    : mesh(std::move(mesh)), material(std::move(material))
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

const std::shared_ptr<graphics::Material>& MeshComponent::getMaterial() const
{
    return material;
}

void MeshComponent::setMaterial(std::shared_ptr<graphics::Material> material_)
{
    this->material = std::move(material_);
}
