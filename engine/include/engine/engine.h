//
// Created by Jonathan Richard on 2024-01-29.
//
#pragma once
#include "renderer/renderer.h"

class engine
{
public:

    explicit engine(const renderer &renderer) : renderer(renderer) {};

    void update(float dt) const;
    void render() const;

private:

    const renderer& renderer;
};