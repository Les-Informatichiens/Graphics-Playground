//
// Created by Jonathan Richard on 2024-02-03.
//

#pragma once

#include "graphicsAPI/common/Common.h"

#include "GraphicsPipelineReflection.h"
#include "graphicsAPI/common/GraphicsPipeline.h"
#include "graphicsAPI/opengl/Context.h"

#include <map>
#include <array>
#include <queue>

namespace opengl
{


struct BlendMode {
    GLenum blendOpColor;
    GLenum blendOpAlpha;
    GLenum srcColor;
    GLenum dstColor;
    GLenum srcAlpha;
    GLenum dstAlpha;
};

class GraphicsPipeline : public IGraphicsPipeline, public WithContext
{
public:
    GraphicsPipeline(Context& context, const GraphicsPipelineDesc& desc_);

    void initialize();

    void bind();
    void unbind();
    void bindTextureUnit(size_t unit, uint8_t bindTarget);
    int getTextureUnitLocation(size_t unit, uint8_t bindTarget);

    void bindTextureSamplerAndUnit(size_t location, uint8_t bindTarget);
    void unbindTextureUnit(size_t location, uint8_t bindTarget);

    void bindVertexAttributes(size_t bufferIndex, size_t offset);
    void unbindVertexAttributes();

    [[nodiscard]] const GraphicsPipelineReflection& getReflection() const { return *reflection; }
    [[nodiscard]] const GraphicsPipelineDesc& getDesc() const override { return this->desc; }

    static GLenum convertBlendOp(BlendOp value);
    static GLenum convertBlendFactor(BlendFactor value);

private:
    GraphicsPipelineDesc desc;

    std::map<uint32_t, std::vector<uint32_t>> bindingAttribLocations;
    std::vector<uint32_t> activeBindingAttribLocations;
    std::array<GLint, MAX_TEXTURE_SAMPLERS> unitSamplerLocations;

//    std::vector<uint32_t> bindingAttribLocations;
    std::shared_ptr<GraphicsPipelineReflection> reflection;

    std::array<GLboolean, 4> colorMask = {GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE};
    BlendMode blendMode = {GL_FUNC_ADD, GL_FUNC_ADD, GL_ONE, GL_ZERO, GL_ONE, GL_ZERO};
    CullMode cullMode = CullMode::None;
    FrontFace frontFace = FrontFace::CounterClockwise;
    PolygonFillMode fillMode = PolygonFillMode::Fill;
    bool blendEnabled = false;
};

}
