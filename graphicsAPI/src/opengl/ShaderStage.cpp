//
// Created by Jonathan Richard on 2024-02-03.
//

#include "ShaderStage.h"
#include "ShaderModule.h"

namespace opengl {

PipelineShaderStages::PipelineShaderStages(Context& context, const PipelineShaderStagesDesc& desc)
    : WithContext(context), desc(desc)
{
}

PipelineShaderStages::~PipelineShaderStages()
{
    if (program != -1)
    {
        getContext().deleteProgram(program);
    }
}

void PipelineShaderStages::createProgram()
{
    switch (desc.type)
    {
    case ShaderStagesType::Graphics:
        createRenderProgram();
        break;
    case ShaderStagesType::Compute:
        createComputeProgram();
        break;
    }
}

void PipelineShaderStages::createRenderProgram()
{
    if (program != -1)
    {
        getContext().deleteProgram(program);
    }

    program = getContext().createProgram();

    GLuint vertexShader = -1;
    GLuint fragmentShader = -1;
    GLuint geometryShader = -1;

    if (desc.vertexModule)
    {
        vertexShader = dynamic_cast<ShaderModule*>(desc.vertexModule.get())->getShader();
    }
    if (desc.fragmentModule)
    {
        fragmentShader = dynamic_cast<ShaderModule*>(desc.fragmentModule.get())->getShader();
    }
    if (desc.geometryModule)
    {
        geometryShader = dynamic_cast<ShaderModule*>(desc.geometryModule.get())->getShader();
    }

    if (vertexShader != -1)
    {
        getContext().attachShader(program, vertexShader);
    }
    if (fragmentShader != -1)
    {
        getContext().attachShader(program, fragmentShader);
    }
    if (geometryShader != -1)
    {
        getContext().attachShader(program, geometryShader);
    }

    getContext().linkProgram(program);

    GLint success = 0;
    getContext().getProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[512];
        getContext().getProgramInfoLog(program, 512, nullptr, infoLog);
        getContext().deleteProgram(program);
        program = -1;
        throw std::runtime_error("Shader program linking failed: " + std::string(infoLog));
    }

    if (vertexShader != -1)
    {
        getContext().detachShader(program, vertexShader);
    }
    if (fragmentShader != -1)
    {
        getContext().detachShader(program, fragmentShader);
    }
    if (geometryShader != -1)
    {
        getContext().detachShader(program, geometryShader);
    }
}

void PipelineShaderStages::createComputeProgram()
{
    if (program != -1)
    {
        getContext().deleteProgram(program);
    }

    program = getContext().createProgram();

    GLuint computeShader = -1;

    if (desc.computeModule)
    {
        computeShader = dynamic_cast<ShaderModule*>(desc.computeModule.get())->getShader();
    }

    if (computeShader != -1)
    {
        getContext().attachShader(program, computeShader);
    }

    getContext().linkProgram(program);

    GLint success = 0;
    getContext().getProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[512];
        getContext().getProgramInfoLog(program, 512, nullptr, infoLog);
        getContext().deleteProgram(program);
        program = -1;
        throw std::runtime_error("Shader program linking failed: " + std::string(infoLog));
    }

    if (computeShader != -1)
    {
        getContext().detachShader(program, computeShader);
    }
}

const std::shared_ptr<IShaderModule>& PipelineShaderStages::getVertexShader() const
{
    return desc.vertexModule;
}

const std::shared_ptr<IShaderModule>& PipelineShaderStages::getFragmentShader() const
{
    return desc.fragmentModule;
}

const std::shared_ptr<IShaderModule>& PipelineShaderStages::getGeometryShader() const
{
    return desc.geometryModule;
}

const std::shared_ptr<IShaderModule>& PipelineShaderStages::getComputeShader() const
{
    return desc.computeModule;
}

void PipelineShaderStages::bind()
{
    if (program != -1)
    {
        getContext().useProgram(program);
    }
}

void PipelineShaderStages::unbind()
{
    if (program != -1)
    {
        getContext().useProgram(0);
    }
}


}; // namespace opengl