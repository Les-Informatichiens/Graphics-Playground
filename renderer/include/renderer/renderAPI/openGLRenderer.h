//
// Created by Jean on 1/30/2024.
//
#pragma once

#include "renderer/renderer.h"

class openGLRenderer : public renderer {
public:
    void init() override{
        // Initialize OpenGL
    }

    void clear() const override  {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void draw(const vertexBuffer& vertexBuffer) const override  {
        // Bind VertexArray and issue draw call
    }
};
