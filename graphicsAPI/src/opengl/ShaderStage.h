//
// Created by Jonathan Richard on 2024-02-03.
//

#pragma once

#include "graphicsAPI/common/ShaderStage.h"
#include "graphicsAPI/opengl/Context.h"

namespace opengl {

class PipelineShaderStages : public IPipelineShaderStages, public WithContext
{
public:
    PipelineShaderStages(Context& context, const PipelineShaderStagesDesc& desc);
    ~PipelineShaderStages() override;

    void createProgram();

    [[nodiscard]] const std::shared_ptr<IShaderModule>& getVertexShader() const override;
    [[nodiscard]] const std::shared_ptr<IShaderModule>& getFragmentShader() const override;
    [[nodiscard]] const std::shared_ptr<IShaderModule>& getGeometryShader() const override;
    [[nodiscard]] const std::shared_ptr<IShaderModule>& getComputeShader() const override;

    [[nodiscard]] ShaderStagesType getType() const override { return desc.type; }

    void bind();
    void unbind();

    [[nodiscard]] GLuint getProgram() const { return program; }

private:
    void createRenderProgram();
    void createComputeProgram();

private:
    PipelineShaderStagesDesc desc;

    GLuint program = -1;
};

}// namespace opengl
