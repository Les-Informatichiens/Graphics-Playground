//
// Created by Jonathan Richard on 2024-02-04.
//

#include "GraphicsPipelineReflection.h"
#include "ShaderModule.h"
#include "Texture.h"
//#include "fmt/format.h"
#include <cstring>

namespace opengl {

namespace {

UniformType getUniformTypeFromOpenGL(GLenum type) {
    switch (type) {
        case GL_FLOAT:
            return UniformType::Float;
        case GL_FLOAT_VEC2:
            return UniformType::Float2;
        case GL_FLOAT_VEC3:
            return UniformType::Float3;
        case GL_FLOAT_VEC4:
            return UniformType::Float4;
        case GL_INT:
            return UniformType::Int;
        case GL_INT_VEC2:
            return UniformType::Int2;
        case GL_INT_VEC3:
            return UniformType::Int3;
        case GL_INT_VEC4:
            return UniformType::Int4;
        case GL_BOOL:
            return UniformType::Boolean;
        case GL_BOOL_VEC2:
            return UniformType::Boolean;
        case GL_BOOL_VEC3:
            return UniformType::Boolean;
        case GL_BOOL_VEC4:
            return UniformType::Boolean;
        case GL_FLOAT_MAT2:
            return UniformType::Mat2x2;
        case GL_FLOAT_MAT3:
            return UniformType::Mat3x3;
        case GL_FLOAT_MAT4:
            return UniformType::Mat4x4;
        default:
            return UniformType::Invalid;

    }
}

TextureType getTextureTypeFromOpenGL(GLenum type) {
    switch (type) {
        case GL_SAMPLER_2D:
        case GL_IMAGE_2D:
        case GL_IMAGE_2D_MULTISAMPLE:
            return TextureType::Texture2D;
        case GL_SAMPLER_2D_ARRAY:
        case GL_IMAGE_2D_ARRAY:
        case GL_IMAGE_2D_MULTISAMPLE_ARRAY:
            return TextureType::Texture2DArray;
        case GL_SAMPLER_3D:
        case GL_IMAGE_3D:
            return TextureType::Texture3D;
        case GL_SAMPLER_CUBE:
        case GL_IMAGE_CUBE:
            return TextureType::TextureCube;
        case GL_SAMPLER_EXTERNAL_OES:
//            return TextureType::ExternalImage;
        default:
            return TextureType::Invalid;
    }
}

} // namespace

GraphicsPipelineReflection::GraphicsPipelineReflection(Context& context, const PipelineShaderStages& desc)
{
    generateUniformDictionary(context, desc);
    generateUniformBlocksDictionary(context, desc);
    generateAttributeDictionary(context, desc);
    generateShaderStorageBufferObjectDictionary(context, desc);

    // --------------------------------------------------------------------------------------------

//    auto vertShader = dynamic_cast<ShaderModule*>(desc.getVertexShader().get());
//    auto fragShader = dynamic_cast<ShaderModule*>(desc.getFragmentShader().get());
//    auto geomShader = dynamic_cast<ShaderModule*>(desc.getGeometryShader().get());
//
//    if (vertShader)
//    {
//        reflectShader(vertShader->getReflection());
//    }
//    if (fragShader)
//    {
//        reflectShader(fragShader->getReflection());
//    }
//    if (geomShader)
//    {
//        reflectShader(geomShader->getReflection());
//    }

}

uint32_t GraphicsPipelineReflection::getLocation(const std::string& name) const
{
    const auto uniform = uniformDictionary.find(name);
    if (uniform != uniformDictionary.end())
    {
        return uniform->second.location;
    }

    const auto uniformBlock = uniformBlocksDictionary.find(name);
    if (uniformBlock != uniformBlocksDictionary.end())
    {
        return uniformBlock->second.blockIndex;
    }

    const auto attribute = attributeDictionary.find(name);
    if (attribute != attributeDictionary.end())
    {
        return attribute->second;
    }

    const auto ssbo = shaderStorageBufferObjectDictionary.find(name);
    if (ssbo != shaderStorageBufferObjectDictionary.end())
    {
        return ssbo->second;
    }

    return -1;
}

void GraphicsPipelineReflection::generateUniformDictionary(Context& context, const PipelineShaderStages& desc)
{
    uniformDictionary.clear();

    GLuint pid = desc.getProgram();

    GLint maxUniformNameLength = 0;
    context.getProgramiv(pid, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformNameLength);
    GLint count = 0;
    context.getProgramiv(pid, GL_ACTIVE_UNIFORMS, &count);

    std::vector<GLchar> cname(maxUniformNameLength);
    for (int i = 0; i < count; i++) {
        GLsizei length = 0;
        GLsizei size = 0;
        GLenum type = GL_NONE;

        context.getActiveUniform(pid, i, maxUniformNameLength, &length, &size, &type, cname.data());
        GLint location = context.getUniformLocation(pid, cname.data());
        if (location < 0) {
            // this uniform belongs to a block;
            continue;
        }

        if (length >= 4 && std::strcmp(cname.data() + length - 3, "[0]") == 0) {
            length = length - 3; // remove '[0]' for arrays
        }
        auto name = std::string(cname.data(), cname.data() + length);
        UniformDesc u(size, location, type);
        uniformDictionary.emplace(name, u);
    }
}

void GraphicsPipelineReflection::generateUniformBlocksDictionary(Context& context, const PipelineShaderStages& desc)
{
    uniformBlocksDictionary.clear();

    GLuint pid = desc.getProgram();

    // todo
}

void GraphicsPipelineReflection::generateAttributeDictionary(Context& context, const PipelineShaderStages& desc)
{
    // todo
}

void GraphicsPipelineReflection::generateShaderStorageBufferObjectDictionary(Context& context, const PipelineShaderStages& desc)
{
    shaderStorageBufferObjectDictionary.clear();

    GLuint pid = desc.getProgram();

    GLint maxSSBONameLength = 0;
    context.getProgramInterfaceiv(
            pid, GL_SHADER_STORAGE_BLOCK, GL_MAX_NAME_LENGTH, &maxSSBONameLength);
    GLint count = 0;
    context.getProgramInterfaceiv(pid, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &count);

    std::vector<GLchar> cname(maxSSBONameLength);
    for (int i = 0; i < count; ++i)
    {
        GLsizei length = 0;
        context.getProgramResourceName(pid, GL_SHADER_STORAGE_BLOCK, i, maxSSBONameLength, &length, cname.data());
        GLint index = context.getProgramResourceIndex(pid, GL_SHADER_STORAGE_BLOCK, cname.data());

        if (index == GL_INVALID_INDEX)
        {
            continue;
        }

        shaderStorageBufferObjectDictionary.emplace(std::string(cname.data(), cname.data() + length), index);
    }
}

// --------------------------------------------------------------------------------------------
void GraphicsPipelineReflection::reflectShader(const std::shared_ptr<ShaderModuleReflection>& reflection)
{
    // append all the shader reflection data to the pipeline reflection
    for (const auto& [k, v] : reflection->getUniformBufferBindingMap())
    {
        uniformBufferBindingMap.try_emplace(k, v);
    }
    for (const auto& [k, v] : reflection->getStorageImageBindingMap())
    {
        storageImageBindingMap.try_emplace(k, v);
    }
    for (const auto& [k, v] : reflection->getSamplerBindingMap())
    {
        samplerBindingMap.try_emplace(k, v);
    }

    for (const auto& [k, v] : reflection->getUniformBufferResources())
    {
        uniformBufferResources.try_emplace(k, v);
    }
    for (const auto& [k, v] : reflection->getStorageImageResources())
    {
        storageImageResources.try_emplace(k, v);
    }
    for (const auto& [k, v] : reflection->getSamplerResources())
    {
        samplerResources.try_emplace(k, v);
    }


    for (const auto& [k, v] : reflection->getPushConstants())
    {
        pushConstants.try_emplace(k, v);
    }
}
GLuint GraphicsPipelineReflection::getSamplerBinding(uint32_t resourceId, bool exceptionIfNotFound) const
{
    const auto resource = uniformBufferResources.find(resourceId);
    GLuint retVal = (resource == uniformBufferResources.end()) ? ~0 : resource->second.binding;
    if (exceptionIfNotFound && retVal == ~0) {
//        throw std::runtime_error(fmt::format("Sampler resource not found: {}", resourceId));
    }
    return retVal;
}

GLuint GraphicsPipelineReflection::getStorageImageBinding(uint32_t resourceId, bool exceptionIfNotFound) const
{
    const auto resource = storageImageResources.find(resourceId);
    GLuint retVal = (resource == storageImageResources.end()) ? ~0 : resource->second.binding;
    if (exceptionIfNotFound && retVal == ~0) {
//        throw std::runtime_error(fmt::format("Storage image resource not found: {}", resourceId));
    }
    return retVal;
}

GLuint GraphicsPipelineReflection::getUniformBufferBinding(uint32_t resourceId, bool exceptionIfNotFound) const
{
    const auto resource = uniformBufferResources.find(resourceId);
    GLuint retVal = (resource == uniformBufferResources.end()) ? ~0 : resource->second.binding;
    if (exceptionIfNotFound && retVal == ~0) {
//        throw std::runtime_error(fmt::format("Uniform buffer resource not found: {}", resourceId));
    }
    return retVal;
}

}// namespace opengl