//
// Created by Jean on 1/30/2024.
//
#pragma once

#include "renderer/iRenderer.h"
#include "GL/glew.h"

class openGLRenderer : public iRenderer {
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
