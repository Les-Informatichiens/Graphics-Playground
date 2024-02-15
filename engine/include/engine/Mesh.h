//
// Created by Jonathan Richard on 2024-02-12.
//

#pragma once

#include "glm/glm.hpp"
#include <vector>

class Mesh {
public:

    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoords;
    };

public:

    Mesh() = default;
    Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices);

    void normalize();

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};
