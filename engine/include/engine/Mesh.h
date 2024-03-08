//
// Created by Jonathan Richard on 2024-02-12.
//

#pragma once

#include "glm/glm.hpp"
#include <vector>
#include <memory>

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

    static std::shared_ptr<Mesh> createCube(float size = 1.0f);
    static std::shared_ptr<Mesh> createQuad(float size = 1.0f);

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};
