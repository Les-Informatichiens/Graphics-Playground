//
// Created by Jonathan Richard on 2024-03-26.
//

#pragma once

#include "glm/vec3.hpp"

enum class LightType
{
    Directional = 0,
    Point = 1,
    Spot = 2
};

class Light
{
    friend class SceneEditor;
public:
    Light() = default;
    ~Light() = default;

    void setIntensity(float intensity)
    {
        this->intensity = intensity;
    }

    [[nodiscard]] float getIntensity() const
    {
        return intensity;
    }

    [[nodiscard]] LightType getType() const
    {
        return type;
    }

    void setAttenuation(float constant, float linear, float quadratic)
    {
        this->constant = constant;
        this->linear = linear;
        this->quadratic = quadratic;
    }

    void setSpot(float cutOff, float outerCutOff)
    {
        this->cutOff = cutOff;
        this->outerCutOff = outerCutOff;
        this->type = LightType::Spot;
    }

    void setPoint()
    {
        this->type = LightType::Point;
    }

    void setDirectional()
    {
        this->type = LightType::Directional;
    }

    [[nodiscard]] float getConstant() const
    {
        return constant;
    }

    [[nodiscard]] float getLinear() const
    {
        return linear;
    }

    [[nodiscard]] float getQuadratic() const
    {
        return quadratic;
    }

    [[nodiscard]] float getCutOff() const
    {
        return cutOff;
    }

    [[nodiscard]] float getOuterCutOff() const
    {
        return outerCutOff;
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
    glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
    float intensity = 1.0f;
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;

    LightType type = LightType::Point;

    // Spot
    float cutOff = 12.5f;
    float outerCutOff = 17.5f;
};
