//
// Created by jeang on 2024-01-25.
//

#pragma once

#include "renderer/renderAPI/vertexBuffer.h"

class iRenderer {
public:
    virtual ~iRenderer() = default;

    virtual void init() = 0;

    virtual void clear() const = 0;

    virtual void draw(const vertexBuffer& vertexBuffer) const = 0;
};