//
// Created by Jean on 1/30/2024.
//
#pragma once

#include "renderer/iRenderer.h"


class openGLRenderer : public iRenderer {
public:
    void init() override{
        // Initialize OpenGL
        glewInit();

    }

    void clear() const override  {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void draw(const vertexBuffer& vertexBuffer) const override  {
        // Bind VertexArray and issue draw call
    }
};
