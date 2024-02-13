//
// Created by Jonathan Richard on 2024-02-04.
//

#pragma once


#include "graphicsAPI/opengl/Context.h"

namespace opengl {

class VertexArrayObject final : public WithContext {
    friend class Device;

public:
    explicit VertexArrayObject(Context& context) : WithContext(context) {}
    ~VertexArrayObject();

    void create();
    void bind() const;
    void unbind() const;

private:
    GLuint vertexAttriuteObject_ = ~0;
};

} // namespace opengl
