//
// Created by Jonathan Richard on 2024-02-13.
//

#pragma once

#include "Mesh.h"
#include "Transform.h"
#include <string>
#include <list>
#include <memory>

class Model
{
public:
    explicit Model(std::string  name);

    ~Model() = default;

    void setTransform(const Transform& transform);
    void setMesh(const Mesh& mesh);

    [[nodiscard]] Transform& getTransform();
    [[nodiscard]] Mesh& getMesh();

protected:
    std::string name;
    Transform transform;
    Mesh mesh;

    Model* parent = nullptr;
//    std::list<std::unique_ptr<Model>> children;
};
