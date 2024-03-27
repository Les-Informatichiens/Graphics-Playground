//
// Created by Jonathan Richard on 2024-03-26.
//

#pragma once

#include "glm/vec3.hpp"

// for now this class is just a container for point light properties
class Light
{
public:
    Light() = default;
    ~Light() = default;

    void setAmbient(float ambient)
    {
        this->ambient = ambient;
    }

    [[nodiscard]] float getAmbient() const
    {
        return ambient;
    }

    void setDiffuse(float diffuse)
    {
        this->diffuse = diffuse;
    }

    [[nodiscard]] float getDiffuse() const
    {
        return diffuse;
    }

    void setSpecular(float specular)
    {
        this->specular = specular;
    }

    [[nodiscard]] float getSpecular() const
    {
        return specular;
    }

    void setColor(float r, float g, float b)
    {
        color = glm::vec3(r, g, b);
    }

    void setColor(glm::vec3 color)
    {
        this->color = color;
    }

    [[nodiscard]] glm::vec3 getColor() const
    {
        return color;
    }

private:
    float ambient = 0.1f;
    float diffuse = 0.5f;
    float specular = 1.0f;
    glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
};
