//
// Created by Jonathan Richard on 2024-02-04.
//

#pragma once

#include "ShaderModuleReflection.h"
#include "ShaderStage.h"
#include "graphicsAPI/common/Texture.h"
#include "graphicsAPI/common/Uniform.h"
#include "graphicsAPI/opengl/Context.h"

namespace opengl
{

/**
 * @brief A structure that describes a buffer argument to a shader.
 */
struct BufferArgDesc {
    /**
   * @brief A structure that describes a member of a buffer argument.
   */
    struct BufferMemberDesc {
        std::string name; ///< The name of the member
        UniformType type = UniformType::Invalid; ///< The type of the member
        size_t offset = 0; ///< The offset from the beginning of the structure
        size_t arrayLength = 1; ///< The number of elements if the member is an array
    };

    std::string name; ///< The name of the buffer argument
    size_t bufferAlignment = 0; ///< The required byte alignment in memory
    size_t bufferDataSize = 0; ///< The size of the buffer argument in bytes
    int bufferIndex = -1; ///< The index of the buffer argument

    ShaderModuleType shaderStage = ShaderModuleType::Fragment; ///< The shader stage the argument belongs to.
    bool isUniformBlock = false; // used in the OpenGL backend only

    std::vector<BufferMemberDesc> members; ///< An array of BufferMemberDesc that describes
                                          ///< each members of a buffer argument.
};

/**
 * @brief A structure that describes the texture argument to a shader.
 */
struct TextureArgDesc {
    std::string name; ///< The name of the texture argument
    TextureType type; ///< The type of the texture argument
    int textureIndex = -1; ///< The index in argument table
    ShaderModuleType shaderStage = ShaderModuleType::Fragment; ///< The shader stage that the texture argument belongs to.
};

/**
 * @brief A structure that describes the texture sampler argument to a shader.
 */
struct SamplerArgDesc {
    std::string name; ///< The name of the sampler argument
    int samplerIndex = -1; ///< The index in the argument table
    ShaderModuleType shaderStage = ShaderModuleType::Fragment; ///< The shader stage that the sampler argument belongs to
};

class GraphicsPipelineReflection
{
public:
    struct UniformDesc {
        GLsizei size;
        GLint location;
        GLenum type;

        UniformDesc(GLsizei s, GLint loc, GLenum t) : size(s), location(loc), type(t) {}
    };

    struct UniformBlockDesc {
        struct UniformBlockMemberDesc {
            GLsizei size = 0;
            GLenum type = GL_NONE;
            GLint offset = 0;
        };

        GLint size;
        GLint blockIndex;
        GLint bindingIndex = -1; // the block binding location, when set directly in the shader
        std::unordered_map<std::string, UniformBlockMemberDesc> members;
    };

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

    uint32_t getLocation(const std::string& name) const;

private:
    void generateUniformDictionary(Context& context, const PipelineShaderStages& desc);
    void generateUniformBlocksDictionary(Context& context, const PipelineShaderStages& desc);
    void generateAttributeDictionary(Context& context, const PipelineShaderStages& desc);

    // --------------------------------------------------------------------------------------------

public:
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
    std::unordered_map<std::string, UniformDesc> uniformDictionary;
    std::unordered_map<std::string, UniformBlockDesc> uniformBlocksDictionary;
    std::unordered_map<std::string, uint32_t> attributeDictionary;
    std::unordered_map<std::string, uint32_t> shaderStorageBufferObjectDictionary;

    // --------------------------------------------------------------------------------------------

    OpenGLUniformMap pushConstants;
    OpenGLBindingMap uniformBufferBindingMap;
    OpenGLResourceMap uniformBufferResources;// maps resource id (essentially the name of the resource) -> its opengl binding
    OpenGLBindingMap samplerBindingMap;
    OpenGLResourceMap samplerResources;// maps resource id (essentially the name of the resource) -> its opengl binding
    OpenGLBindingMap storageImageBindingMap;
    OpenGLResourceMap storageImageResources;

};

}
