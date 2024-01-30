//
// Created by Jonathan Richard on 2024-01-29.
//
#pragma once
#include "renderer/renderer.h"

class engine
{
public:
    void update(float dt);
    void render();

private:

    renderer renderer{};
};