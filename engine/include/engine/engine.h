//
// Created by Jonathan Richard on 2024-01-29.
//
#pragma once
#include "renderer/iRenderer.h"
#include <utility>
#include <memory>

class engine
{
public:

    explicit engine(iRenderer& renderer) : renderer(renderer) {};

    void update(float dt) const;
    void render() const;

private:

    iRenderer& renderer;
};