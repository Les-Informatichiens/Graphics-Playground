//
// Created by Jonathan Richard on 2024-02-12.
//

#pragma once

#include <engine/Bounds.h>

#include "engine/util/UUID.h"
#include "glm/glm.hpp"
#include <memory>
#include <vector>

class Mesh {
public:

    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoords;
//        glm::vec3 tangent;
//        glm::vec3 bitangent;
    };

public:

    Mesh() = default;
    Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices);

    void normalize();
//    void calculateTangents();

    static std::shared_ptr<Mesh> createCube(float size = 1.0f);
    static std::shared_ptr<Mesh> createQuad(float size = 1.0f);
    static std::shared_ptr<Mesh> createSphere(float radius = 1.0f, unsigned int longitudeSegments = 20, unsigned int latitudeSegments = 20);

    void recalculateBounds() const;

    void clear();

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    mutable Bounds bounds = Bounds(glm::vec3(0.0f), glm::vec3(0.0f));
};
