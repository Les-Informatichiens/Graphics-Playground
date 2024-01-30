//
// Created by Jonathan Richard on 2024-01-29.
//

#include "engine/engine.h"

#include <iostream>

void engine::update(float dt) const
{
    //renderer.draw();
    std::cout << "Engine updated in " << dt << "ms" << std::endl;
}

void engine::render() const {
    std::cout << "Engine rendered" << std::endl;
}