//
// Created by Jonathan Richard on 2024-03-07.
//

#pragma once

#include "engine/graphics/Renderer.h"
#include "glm/glm.hpp"
#include <vector>


struct LineSettings
{
    float aspect = 1.0f;
    float lineWidth = 1.0f;
    int miter = 0;
    glm::vec4 color = glm::vec4(1.0f);
};

class LineRenderer
{
public:
    LineRenderer() = default;
    ~LineRenderer() = default;

    void setAspect(float aspect) { settings.aspect = aspect; };
    void setLineWidth(float lineWidth) { settings.lineWidth = lineWidth; };
    void setMiter(int miter) { settings.miter = miter; };
    void setColor(const glm::vec4& color) { settings.color = color; };
    void setSettings(const LineSettings& settings) { this->settings = settings; };

    void setVP(const glm::mat4& view, const glm::mat4& projection)
    {
        this->view = view;
        this->projection = projection;
    };

    void drawLine(graphics::Renderer& renderer, const glm::vec3& start, const glm::vec3& end);

    void drawLines(graphics::Renderer& renderer, const std::vector<glm::vec3>& lines);

private:
    std::vector<uint16_t> createIndices(uint32_t length);

private:
    glm::mat4 view;
    glm::mat4 projection;

    std::vector<glm::vec3> lineVertices;
    std::vector<glm::vec3> lineColors;
    std::vector<uint16_t> lineIndices;
    static std::shared_ptr<graphics::VertexData> lineVertexData;
    static std::shared_ptr<graphics::Material> lineMaterial;

    LineSettings settings{};
};