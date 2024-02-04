//
// Created by Jonathan Richard on 2024-02-04.
//

#pragma once

#include "graphicsAPI/common/VertexInputState.h"
#include "graphicsAPI/opengl/Context.h"

#include <map>
#include <vector>

namespace opengl {

struct OpenGLAttributeDesc
{
    std::string name;
    GLsizei stride = 0;
    uintptr_t offset = 0;
    GLint numComponents = 0;
    GLenum type = GL_FLOAT;
    GLboolean normalized = false;
};

class VertexInputState : public IVertexInputState
{
public:
    explicit VertexInputState(const VertexInputStateDesc& desc);

    [[nodiscard]] const std::vector<OpenGLAttributeDesc>& getBufferAttributes(size_t bufferIndex = 0) const;
    [[nodiscard]] const std::map<size_t, std::vector<OpenGLAttributeDesc>>& getBufferAttribMap() const { return bufferAttribMap; }

private:
    void populateBufferAttributes(const VertexInputStateDesc& desc);

private:
    std::map<size_t, std::vector<OpenGLAttributeDesc>> bufferAttribMap;
};

}// namespace OpenGL
