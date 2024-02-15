//
// Created by Jonathan Richard on 2024-02-13.
//

#include "engine/Model.h"

#include <utility>

Model::Model(std::string name)
    : name(std::move(name))
{
}

void Model::setTransform(const Transform& transform)
{
    this->transform = transform;
}

void Model::setMesh(const Mesh& mesh)
{
    this->mesh = mesh;
}

Transform& Model::getTransform()
{
    return this->transform;
}

Mesh& Model::getMesh()
{
    return this->mesh;
}
