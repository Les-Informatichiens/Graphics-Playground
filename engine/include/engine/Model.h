//
// Created by Jonathan Richard on 2024-02-13.
//

#pragma once

#include "Mesh.h"
#include "Transform.h"
#include <string>

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
};
