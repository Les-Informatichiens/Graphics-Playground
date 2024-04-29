//
// Created by Jonathan Richard on 2024-03-07.
//

#include "engine/LineRenderer.h"
#include "engine/util/Math.h"
#include <algorithm>

// declare static member
std::shared_ptr<graphics::Material> LineRenderer::lineMaterial;
std::shared_ptr<graphics::VertexData> LineRenderer::lineVertexData;

void LineRenderer::drawLine(graphics::Renderer& renderer, const glm::vec3& start, const glm::vec3& end)
{
    std::vector<glm::vec3> lines = {start, end};
    drawLines(renderer, lines);
}

/*
 * source: https://mattdesl.svbtle.com/drawing-lines-is-hard
 */
void LineRenderer::drawLines(graphics::Renderer& renderer, const std::vector<glm::vec3>& lines)
{
//    lineVertices.clear();
//    lineIndices.clear();
//
//    lineVertices.reserve(lines.size());
//    lineIndices.reserve(lines.size());

    if (!LineRenderer::lineMaterial)
    {
    // Create line material
    auto vs = R"(
        #version 450 core

        layout(location = 0) in vec3 position;
        layout(location = 1) in float direction;
        layout(location = 2) in vec3 next;
        layout(location = 3) in vec3 previous;

        layout(binding = 0) uniform MVP
        {
            mat4 model;
            mat4 view;
            mat4 projection;
        } mvp;

        layout(binding = 1) uniform Constants
        {
            float aspect;
            float thickness;
            int miter;
            vec4 color;
        } constants;

        void main()
        {
            vec2 aspectVec = vec2(constants.aspect, 1.0);
            mat4 projViewModel = mvp.projection * mvp.view * mvp.model;
            vec4 previousProjected = projViewModel * vec4(previous, 1.0);
            vec4 currentProjected = projViewModel * vec4(position, 1.0);
            vec4 nextProjected = projViewModel * vec4(next, 1.0);

            //get 2D screen space with W divide and aspect correction
            vec2 currentScreen = (currentProjected.xy / currentProjected.w);
            vec2 previousScreen = (previousProjected.xy / previousProjected.w);
            vec2 nextScreen = (nextProjected.xy / nextProjected.w);
            currentScreen.x *= constants.aspect;
            previousScreen.x *= constants.aspect;
            nextScreen.x *= constants.aspect;

            float len = constants.thickness;
            float orientation = direction;

            float miterLimit = 1.0;
            //starting point uses (next - current)
            vec2 dir = vec2(0.0);
            if (currentScreen == previousScreen) {
              dir = normalize(nextScreen - currentScreen);
            }
            //ending point uses (current - previous)
            else if (currentScreen == nextScreen) {
              dir = normalize(currentScreen - previousScreen);
            }
            //somewhere in middle, needs a join
            else {
              //get directions from (C - B) and (B - A)
              vec2 dirA = normalize((currentScreen - previousScreen));
                if (constants.miter == 1) {
                    vec2 dirB = normalize((nextScreen - currentScreen));
                    //now compute the miter join normal and length
                    vec2 tangent = normalize(dirA + dirB);
                    vec2 perp = vec2(-dirA.y, dirA.x);
                    vec2 miter = vec2(-tangent.y, tangent.x);
                    dir = tangent;
                    float dotProduct = dot(miter, perp);
                    if (abs(dotProduct) > 0.0001) { // Add this check
                        float potentialMiterLength = constants.thickness / dotProduct;
                        if (potentialMiterLength > miterLimit) {
                            len = constants.thickness;
                        } else {
                            len = potentialMiterLength;
                        }
                    } else {
                        len = constants.thickness;
                    }
                } else {
                    dir = dirA;
                }
            }
            len *= currentProjected.w;
            vec2 normal = vec2(-dir.y, dir.x);
            normal *= len/2.0;
            normal.x /= constants.aspect;

            vec4 offset = vec4(normal * orientation, 0.0, 0.0);
            gl_Position = currentProjected + offset;
            gl_PointSize = 1.0;
        }
    )";

    auto fs = R"(
        #version 450 core

        layout(binding = 1) uniform Constants
        {
            float aspect;
            float thickness;
            int miter;
            vec4 color;
        } constants;

        out vec4 fragColor;

        void main()
        {
            fragColor = constants.color;
        }
    )";


        auto shaderProgram = renderer.getDeviceManager().createShaderProgram(vs, fs);
        LineRenderer::lineMaterial = renderer.getDeviceManager().createMaterial(shaderProgram);

    }

    LineRenderer::lineMaterial->setCullMode(CullMode::None);
    LineRenderer::lineMaterial->setDepthTestConfig(graphics::DepthTestConfig::Enable);

    // struct Constants
    // {
    //     alignas(4) float aspect = 1.0f;
    //     alignas(4) float thickness = 1.0f;
    //     alignas(4) int miter = 0;
    //     alignas(16) glm::vec4 color;
    // } constants = {
    //     .aspect = settings.aspect,
    //     .thickness = settings.lineWidth,
    //     .miter = settings.miter,
    //     .color = settings.color
    // };
    //
    // struct MVP
    // {
    //     glm::mat4 model = glm::mat4(1.0f);
    //     glm::mat4 view = glm::mat4(1.0f);
    //     glm::mat4 projection = glm::mat4(1.0f);
    // } mvp = {
    //     .model = glm::mat4(1.0f),
    //     .view = view,
    //     .projection = projection
    // };
    //
    // lineMaterial->setUniformBytes("MVP", &mvp, sizeof(mvp), 0);
    // lineMaterial->setUniformBytes("Constants", &constants, sizeof(constants), 1);
    //
    // // Create vertex data
    // graphics::VertexDataLayout attribLayout({
    //     {"position", 0, VertexAttributeFormat::Float3},
    //     {"direction", 1, VertexAttributeFormat::Float},
    //     {"next", 2, VertexAttributeFormat::Float3},
    //     {"previous", 3, VertexAttributeFormat::Float3}
    // });
    //
    // auto indices = createIndices(lines.size());
    //
    // struct LineVertex
    // {
    //     glm::vec3 position = glm::vec3(0.0f);
    //     float direction = 1.0f;
    //     glm::vec3 next = glm::vec3(0.0f);
    //     glm::vec3 previous = glm::vec3(0.0f);
    // };
    //
    // auto relative = [](int offset, const glm::vec3& point, int index, const std::vector<glm::vec3>& list)
    // {
    //     index = std::clamp(index + offset, 0, static_cast<int>(list.size() - 1));
    //     return list[index];
    // };
    //
    // std::vector<LineVertex> vertices;
    // // Process line points into vertices
    // for (size_t i = 0; i < lines.size(); i++)
    // {
    //     glm::vec3 current = lines[i];
    //     glm::vec3 previous = relative(-1, current, i, lines);
    //     glm::vec3 next = relative(1, current, i, lines);
    //
    //     LineVertex v1;
    //     v1.position = current;
    //     v1.direction = 1.0f;
    //     v1.next = next;
    //     v1.previous = previous;
    //
    //     LineVertex v2;
    //     v2.position = current;
    //     v2.direction = -1.0f;
    //     v2.next = next;
    //     v2.previous = previous;
    //
    //     vertices.push_back(v1);
    //     vertices.push_back(v2);
    // }
    // if (!lineVertexData)
    // {
    //     lineVertexData = renderer.getDeviceManager().createIndexedVertexData(attribLayout, IndexFormat::UInt16, vertices.size(), indices.size());
    // }
    // else
    // {
    //     lineVertexData->allocateVertexBuffer(renderer.getDevice(), vertices.size());
    //     lineVertexData->allocateIndexBuffer(renderer.getDevice(), indices.size());
    // }
    // lineVertexData->pushIndices(indices);
    // lineVertexData->pushVertices(vertices);
    //
    // graphics::Renderable lineRenderable(LineRenderer::lineMaterial, lineVertexData);
    // lineRenderable.setElementCount((lines.size()-1)*6);
    // renderer.draw(lineRenderable);


}

std::vector<uint16_t> LineRenderer::createIndices(uint32_t length)
{
    lineIndices.resize(length * 6);
    int c = 0, index = 0;
    for (size_t j = 0; j < length; j++)
    {
        int i = index;
        lineIndices[c++] = i;
        lineIndices[c++] = i + 1;
        lineIndices[c++] = i + 2;
        lineIndices[c++] = i + 2;
        lineIndices[c++] = i + 1;
        lineIndices[c++] = i + 3;
        index += 2;
    }
    return lineIndices;
}
