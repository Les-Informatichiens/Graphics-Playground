//
// Created by Jonathan Richard on 2024-03-26.
//

#pragma once

#include "engine/Light.h"

class LightComponent
{
public:
    LightComponent(Light light)
    {
        this->light = light;
    }

    [[nodiscard]] Light* getLight()
    {
        return &light;
    }

    [[nodiscard]] const Light* getLight() const
    {
        return &light;
    }

    void setLight(Light light)
    {
        this->light = light;
    }

private:
    Light light;
};
