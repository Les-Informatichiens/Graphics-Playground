//
// Created by jeang on 2024-01-25.
//

#pragma once

#include "renderer/renderAPI/vertexBuffer.h"

class renderer {
public:
    virtual ~renderer() = default;

    virtual void init() = 0;

    virtual void clear() const = 0;

    virtual void draw(const vertexBuffer& vertexBuffer) const = 0;
};