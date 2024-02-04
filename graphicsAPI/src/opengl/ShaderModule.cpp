//
// Created by Jonathan Richard on 2024-02-03.
//

#include "ShaderModule.h"
#include <stdexcept>

namespace opengl {

static GLenum ShaderTypeToOpenGLType(ShaderModuleType type) {
    switch (type) {
        case ShaderModuleType::Vertex:   return GL_VERTEX_SHADER;
        case ShaderModuleType::Geometry: return GL_GEOMETRY_SHADER;
        case ShaderModuleType::Fragment: return GL_FRAGMENT_SHADER;
        case ShaderModuleType::Compute:  return GL_COMPUTE_SHADER;
    }
    return 0;
}

ShaderModule::ShaderModule(Context& context, const ShaderModuleDesc& desc)
    : WithContext(context), IShaderModule(desc), shaderType(ShaderTypeToOpenGLType(desc.type))
{
}

ShaderModule::~ShaderModule()
{
    if (shader != -1)
    {
        getContext().deleteShader(shader);
    }
}

std::string ShaderModule::compileAndParseGLSL(spirv_cross::Compiler& compiler)
{
    std::string glsl = compiler.compile();

    reflection = std::make_shared<ShaderModuleReflection>(compiler);

    return glsl;
}

void ShaderModule::create(const std::string& glsl)
{
    if (isCompiled)
        return;

    GLuint tempShader = getContext().createShader(shaderType);

    const GLchar* code = glsl.data();
    getContext().shaderSource(shader, 1, &code, nullptr);
    getContext().compileShader(shader);

    GLint success = 0;
    getContext().getShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[512];
        getContext().getShaderInfoLog(shader, 512, nullptr, infoLog);
        throw std::runtime_error("Shader compilation failed: " + std::string(infoLog));
    }
    shader = tempShader;
    isCompiled = true;
}

}// namespace opengl
