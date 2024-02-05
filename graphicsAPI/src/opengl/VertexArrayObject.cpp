//
// Created by Jonathan Richard on 2024-02-04.
//

#include "VertexArrayObject.h"

namespace opengl {

VertexArrayObject::~VertexArrayObject()
{
    if (vertexAttriuteObject_ != ~0)
    {
        getContext().deleteVertexArrays(1, &vertexAttriuteObject_);
    }
}

void VertexArrayObject::create()
{
    getContext().genVertexArrays(1, &vertexAttriuteObject_);
}

void VertexArrayObject::bind() const
{
    getContext().bindVertexArray(vertexAttriuteObject_);
}

void VertexArrayObject::unbind() const
{
    getContext().bindVertexArray(0);
}

} // namespace opengl
