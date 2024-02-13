//
// Created by Jonathan Richard on 2024-02-03.
//

#pragma once


#include "graphicsAPI/common/ShaderModule.h"

#include "ShaderModuleReflection.h"
#include "graphicsAPI/opengl/Context.h"

#include <memory>

namespace opengl {

class ShaderModule : public IShaderModule, public WithContext
{
public:
    ShaderModule(Context& context, const ShaderModuleDesc& desc);
    ~ShaderModule() override;

//    std::string compileAndParseGLSL(spirv_cross::Compiler& compiler);
    void create(const std::string& glsl);

    [[nodiscard]] std::shared_ptr<ShaderModuleReflection> getReflection() const { return reflection; }

    GLenum getShaderType() const { return shaderType; }
    GLuint getShader() const { return shader; }

private:
    GLenum shaderType;
    GLuint shader = -1;

    std::shared_ptr<ShaderModuleReflection> reflection;

    bool isCompiled = false;
};

}// namespace opengl
