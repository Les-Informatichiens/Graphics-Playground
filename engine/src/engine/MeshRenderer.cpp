//
// Created by Jonathan Richard on 2024-02-12.
//

#include "engine/MeshRenderer.h"
#include "engine/Transform.h"
#include "glm/gtc/matrix_transform.hpp"

void MeshRenderer::render(graphics::Renderer& renderer, const Mesh& mesh, const Transform& transform)
{

    auto& device = renderer.getDevice();

    auto vs = device.createShaderModule({
            .type = ShaderModuleType::Vertex,
            .code = R"(
                #version 450
                layout(location = 0) in vec3 inPosition;
                layout(location = 1) in vec3 inNormal;
                layout(location = 2) in vec2 inTexCoord;

                layout(location = 0) out vec3 fragNormal;

                layout(binding = 0) uniform UBO {
                    mat4 model;
                    mat4 view;
                    mat4 proj;
                } ubo;

                void main() {
                    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
                    mat3 normalMatrix = transpose(inverse(mat3(ubo.model)));
                    fragNormal = normalMatrix * inNormal;
                }
            )"
    });

    auto fs = device.createShaderModule({
            .type = ShaderModuleType::Fragment,
            .code = R"(
                #version 450
                layout(location = 0) in vec3 fragNormal;
                out vec4 fragColor;

                layout(binding = 1) uniform Constants {
                    vec3 lightDir;
                    float shininess;
                } constants;

                void main() {
                    // shade the fragment based on the normal
                    vec3 baseColor = vec3(0.6, 0.1, 0.1);
                    vec3 shineColor = vec3(1.0);

                    float intensity1 = pow(max(dot(normalize(fragNormal), normalize(constants.lightDir)) + 0.01, 0.025), constants.shininess);
                    float intensity2 = min(pow(max(dot(normalize(fragNormal), normalize(constants.lightDir)), 0), 1.0), 1.0);
                    float intensity = intensity1 + intensity2;
                    vec3 finalColor = intensity1 * shineColor + intensity2 * baseColor;
                    fragColor = vec4(finalColor, 1.0);
                }
            )"
    });

    auto shaderStages = device.createPipelineShaderStages(PipelineShaderStagesDesc::fromRenderModules(vs, fs));


    auto vertexInputState = device.createVertexInputState(
            VertexInputStateDescBuilder()
                    .beginBinding(0)
                    .addVertexAttribute(VertexAttributeFormat::Float3, "inPosition", 0)
                    .addVertexAttribute(VertexAttributeFormat::Float3, "inNormal", 1)
                    .addVertexAttribute(VertexAttributeFormat::Float2, "inTexCoord", 2)
                    .endBinding()
                    .build());

    auto pipeline = device.createGraphicsPipeline({
            .shaderStages = shaderStages,
            .vertexInputState = vertexInputState,
    });

    auto depthStencilState = device.createDepthStencilState({
            .depth = {
                    .writeEnabled = true,
                    .compareOp = CompareOp::LessOrEqual
            },
    });

    std::shared_ptr vertexBuffer = device.createBuffer({
            .type = BufferDesc::BufferTypeBits::Vertex,
            .data = mesh.vertices.data(),
            .size = static_cast<uint32_t>(mesh.vertices.size() * sizeof(Mesh::Vertex)),
            .storage = ResourceStorage::Shared
    });
    std::shared_ptr indexBuffer = device.createBuffer({
            .type = BufferDesc::BufferTypeBits::Index,
            .data = mesh.indices.data(),
            .size = static_cast<uint32_t>(mesh.indices.size() * sizeof(uint32_t)),
            .storage = ResourceStorage::Shared
    });


    struct UniformBufferObject {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

    struct Constants {
        glm::vec3 lightDir;
        float shininess = 32.0f;
    };

    std::shared_ptr uniformBuffer = device.createBuffer({
            .type = BufferDesc::BufferTypeBits::Uniform,
            .size = sizeof(UniformBufferObject),
            .storage = ResourceStorage::Shared
    });

    std::shared_ptr constantsUBO = device.createBuffer({
            .type = BufferDesc::BufferTypeBits::Uniform,
            .size = sizeof(Constants),
            .storage = ResourceStorage::Shared
    });

    auto& activeCamera = renderer.getCamera();

    UniformBufferObject ubo = {
            .model = transform.getModel(),
            .view = activeCamera.getView(),
            .proj = activeCamera.getProjection()
    };

    uniformBuffer->data(&ubo, sizeof(ubo), 0);


    Constants constants = {
            .lightDir = glm::vec3(-0.7f, -0.5f, -0.5f),
            .shininess = 320.0f
    };

    constantsUBO->data(&constants, sizeof(constants), 0);


    graphics::Renderable meshRenderable(pipeline);
    meshRenderable.setVertexData(vertexBuffer, indexBuffer, mesh.indices.size());
    meshRenderable.setUniformBuffer(0, uniformBuffer);
    meshRenderable.setUniformBuffer(1, constantsUBO);
    meshRenderable.setDepthStencilState(depthStencilState);



    renderer.draw(meshRenderable);
}
