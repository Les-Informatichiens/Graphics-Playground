//
// Created by Jonathan Richard on 2024-02-04.
//

#pragma once

#include "ShaderModuleReflection.h"
#include "ShaderStage.h"
#include "graphicsAPI/opengl/Context.h"

namespace opengl
{

class GraphicsPipelineReflection
{
public:

//    struct UniformDesc
//    {
//        GLint location;
//        GLenum type;
//        GLsizei size;
//    };
//
//    struct UniformBlockMemberDesc
//    {
//        GLenum type;
//        GLsizei size;
//        GLint offset;
//    };
//
//    struct UniformBlockDesc
//    {
//        GLint location;
//        GLuint binding;
//        GLsizei size;
//        std::vector<UniformBlockMemberDesc> uniforms;
//    };

    GraphicsPipelineReflection(Context& context, const PipelineShaderStages& desc);

    [[nodiscard]] const OpenGLUniformMap& getPushConstants() const { return pushConstants; }
    [[nodiscard]] const OpenGLBindingMap& getUniformBufferBindingMap() const { return uniformBufferBindingMap; }
    [[nodiscard]] const OpenGLResourceMap& getUniformBufferResources() const { return uniformBufferResources; }
    [[nodiscard]] const OpenGLBindingMap& getSamplerBindingMap() const { return samplerBindingMap; }
    [[nodiscard]] const OpenGLResourceMap& getSamplerResources() const { return samplerResources; }
    [[nodiscard]] const OpenGLBindingMap& getStorageImageBindingMap() const { return storageImageBindingMap; }
    [[nodiscard]] const OpenGLResourceMap& getStorageImageResources() const { return storageImageResources; }

    GLuint getSamplerBinding(uint32_t resourceId, bool exceptionIfNotFound = true) const;
    GLuint getStorageImageBinding(uint32_t resourceId, bool exceptionIfNotFound = true) const;
    GLuint getUniformBufferBinding(uint32_t resourceId, bool exceptionIfNotFound = true) const;

private:
    void reflectShader(const std::shared_ptr<ShaderModuleReflection>& reflection);

private:
    OpenGLUniformMap pushConstants;
    OpenGLBindingMap uniformBufferBindingMap;
    OpenGLResourceMap uniformBufferResources;// maps resource id (essentially the name of the resource) -> its opengl binding
    OpenGLBindingMap samplerBindingMap;
    OpenGLResourceMap samplerResources;// maps resource id (essentially the name of the resource) -> its opengl binding
    OpenGLBindingMap storageImageBindingMap;
    OpenGLResourceMap storageImageResources;

};

}
