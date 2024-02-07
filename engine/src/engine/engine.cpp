//
// Created by Jonathan Richard on 2024-01-29.
//

#include "engine/engine.h"
#include "glm/glm.hpp"

#include <iostream>

engine::engine(IDevice& graphicsDevice) : device(graphicsDevice) {
}

void engine::update(float dt) const
{
    //renderer.draw();
    std::cout << "Engine updated in " << dt << "ms" << std::endl;
}
template<typename T>
using Ref = std::shared_ptr<T>;

void engine::render() const {


    Ref<IShaderModule> vs = device.createShaderModule({
        .type = ShaderModuleType::Vertex,
        .code = R"(#version 310 es
            precision mediump float;
            layout(location = 0) in vec3 inPosition;
            layout(location = 1) in vec3 inColor;

            layout(location = 0) out vec3 fragColor;

            layout(std140, binding = 0) uniform TEST {
                vec4 color;
            } test;

            void main() {
                gl_Position = vec4(inPosition, 1.0);
                fragColor = test.color.rgb * inColor;
            }
        )"
    });

    Ref<IShaderModule> fs = device.createShaderModule({
        .type = ShaderModuleType::Fragment,
        .code = R"(#version 310 es
            precision mediump float;
            layout(location = 0) in vec3 fragColor;

            layout(location = 0) out vec4 outColor;

            void main() {
                outColor = vec4(fragColor, 1.0);
            }
        )"
    });

    Ref<IPipelineShaderStages> shaderStages = device.createPipelineShaderStages(PipelineShaderStagesDesc::fromRenderModules(vs, fs));

    VertexInputStateDesc vertexInputStateDesc = VertexInputStateDescBuilder()
                                        .beginBinding(0)
                                            .addVertexAttribute(VertexAttributeFormat::FLOAT3, "inPosition", 0)
                                        .endBinding()
                                        .beginBinding(1)
                                            .addVertexAttribute(VertexAttributeFormat::FLOAT3, "inColor", 1)
                                        .endBinding()
                                        .build();
    Ref<IVertexInputState> vertexInputState = device.createVertexInputState(vertexInputStateDesc);

    Ref<IGraphicsPipeline> pipeline = device.createGraphicsPipeline({
            .shaderStages = shaderStages,
            .vertexInputState = vertexInputState
    });

    struct TestUniform {
        glm::vec4 color;
    };

    float value = std::sin(clock() / 1000.0f);
    // set uniform data
    auto uniformData = TestUniform{
            .color = {value, value, value, value}
    };

    Ref<IBuffer> uniformBuffer = device.createBuffer({
            .type = BufferDesc::BufferTypeBits::Uniform,
            .data = &uniformData,
            .size = sizeof(TestUniform)
    });

    struct Vertex {
        glm::vec3 Pos;
        glm::vec3 Color;
    };

    // 4 vertices for a square to test index buffer
    std::vector<decltype(Vertex::Pos)> positions = {
            {-0.5f, -0.5f, 0.0f},
            {0.5f, -0.5f, 0.0f},
            {0.0f, 0.5f, 0.0f},
            {0.5f, 0.5f, 0.0f}
    };
    std::vector<decltype(Vertex::Color)> colors = {
            {1.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, 1.0f},
            {1.0f, 1.0f, 1.0f}
    };
    std::vector<uint32_t> indices = {
            0, 1, 2,
            2, 1, 3
    };



    Ref<IBuffer> vertexBuffer = device.createBuffer({
            .type = BufferDesc::BufferTypeBits::Vertex,
            .data = positions.data(),
            .size = static_cast<uint32_t>(positions.size() * sizeof(Vertex::Pos))
    });

    Ref<IBuffer> vertexBuffer2 = device.createBuffer({
            .type = BufferDesc::BufferTypeBits::Vertex,
            .data = colors.data(),
            .size = static_cast<uint32_t>(colors.size() * sizeof(Vertex::Color))
    });

    Ref<IBuffer> indexBuffer = device.createBuffer({
            .type = BufferDesc::BufferTypeBits::Index,
            .data = indices.data(),
            .size = static_cast<uint32_t>(indices.size() * sizeof(uint32_t))
    });

    Ref<ICommandPool> commandPool = device.createCommandPool({});
    Ref<ICommandBuffer> commandBuffer = commandPool->acquireCommandBuffer({});

    commandBuffer->beginRenderPass({});
    commandBuffer->bindGraphicsPipeline(pipeline);
    commandBuffer->bindBuffer(0, vertexBuffer, 0);
    commandBuffer->bindBuffer(1, vertexBuffer2, 0);
    commandBuffer->bindBuffer(0, uniformBuffer, 0);
//    commandBuffer->draw(PrimitiveType::Triangle, 0, 3);
    commandBuffer->drawIndexed(PrimitiveType::Triangle, indices.size(), IndexFormat::UInt32, *indexBuffer, 0);
    commandBuffer->endRenderPass();

    commandPool->submitCommandBuffer(commandBuffer);


    std::cout << "Engine rendered" << std::endl;
}