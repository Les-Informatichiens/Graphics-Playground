//
// Created by Jonathan Richard on 2024-02-04.
//

#pragma once

#include "GraphicsPipeline.h"
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

private:


};

}
