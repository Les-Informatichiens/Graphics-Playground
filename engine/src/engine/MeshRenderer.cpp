//
// Created by Jonathan Richard on 2024-02-12.
//

#include "engine/MeshRenderer.h"
#include "engine/Transform.h"
#include "engine/graphics/ShaderProgram.h"
#include "glm/gtc/matrix_transform.hpp"

void MeshRenderer::render(graphics::Renderer& renderer, const Mesh& mesh, const std::shared_ptr<graphics::Material>& material, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection)
{
    auto& device = renderer.getDevice();

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
    } ubo(model, view, projection);

    struct Constants {
        glm::vec3 lightDir;
        float shininess = 32.0f;
    } constants(glm::vec3(-0.7f, -0.5f, -0.5f), 320.0f);

    material->setUniformBytes("ubo", &ubo, sizeof(ubo), 0);
    material->setUniformBytes("constants", &constants, sizeof(constants), 1);
    material->setCullMode(CullMode::Back);
    material->setBlendMode(graphics::BlendMode::Opaque());
    material->setDepthTestConfig(graphics::DepthTestConfig::Enable);

    graphics::Renderable meshRenderable(material);
    meshRenderable.setVertexData(vertexBuffer, indexBuffer, mesh.indices.size());

    renderer.draw(meshRenderable);
}
