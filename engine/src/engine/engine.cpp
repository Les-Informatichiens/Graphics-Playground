//
// Created by Jonathan Richard on 2024-01-29.
//

#include "engine/engine.h"


#include <iostream>

void engine::update(float dt)
{
    renderer.draw_triangle();
    std::cout << "Engine updated with " << dt << "ms" << std::endl;
}

void engine::render()
{
    std::cout << "Engine rendered" << std::endl;
}
