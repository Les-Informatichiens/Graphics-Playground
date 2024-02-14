//
// Created by Jonathan Richard on 2024-02-04.
//

#include "VertexArrayObject.h"

namespace opengl {

VertexArrayObject::~VertexArrayObject()
{
    std::cout << "Deleting VAO: " << vertexAttriuteObject_ << std::endl;
    if (vertexAttriuteObject_ != 0)
    {
        getContext().deleteVertexArrays(1, &vertexAttriuteObject_);
        std::cout << "Deleted VAO: " << vertexAttriuteObject_ << std::endl;
    }
}

void VertexArrayObject::create()
{
    getContext().genVertexArrays(1, &vertexAttriuteObject_);
}

void VertexArrayObject::bind() const
{
    std::cout << "Binding VAO: " << vertexAttriuteObject_ << std::endl;
    getContext().bindVertexArray(vertexAttriuteObject_);
}

void VertexArrayObject::unbind() const
{
    std::cout << "Unbinding VAO: " << vertexAttriuteObject_ << std::endl;
    getContext().bindVertexArray(0);
}

} // namespace opengl
