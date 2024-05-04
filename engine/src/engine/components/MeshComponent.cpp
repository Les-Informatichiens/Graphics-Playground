//
// Created by Jonathan Richard on 2024-03-05.
//

#include <utility>

#include "engine/components/MeshComponent.h"

MeshComponent::MeshComponent(std::shared_ptr<MeshResource> mesh, std::shared_ptr<MaterialResource> material)
    : mesh(std::move(mesh)), material(std::move(material))
{
}

MeshComponent::~MeshComponent() = default;

std::shared_ptr<MeshResource> MeshComponent::getMesh() const
{
    return mesh;
}

std::shared_ptr<MaterialResource> MeshComponent::getMaterial() const
{
    return material;
}

void MeshComponent::setMesh(std::shared_ptr<MeshResource> mesh_)
{
    this->mesh = std::move(mesh_);
}

void MeshComponent::setMaterial(std::shared_ptr<MaterialResource> material_)
{
    this->material = std::move(material_);
}