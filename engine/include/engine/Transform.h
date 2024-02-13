//
// Created by Jonathan Richard on 2024-02-13.
//

#pragma once

#include "glm/glm.hpp"

class Transform
{
public:
    Transform() = default;
    Transform(const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale);
    ~Transform() = default;

    void translate(const glm::vec3 &translation);
    void rotate(const glm::vec3 &rotation);
    void scale(const glm::vec3 &scale);

    void setPosition(const glm::vec3 &position);
    void setRotation(const glm::vec3 &rotation);
    void setScale(const glm::vec3 &scale);

    void reset();

    [[nodiscard]] glm::mat4 getLookAt(const glm::vec3 &target, const glm::vec3 &up) const;

    [[nodiscard]] glm::mat4 getModel() const;

    glm::vec3 position_ = glm::vec3(0.0f);
    glm::vec3 rotation_ = glm::vec3(0.0f);
    glm::vec3 scale_ = glm::vec3(1.0f);
};
