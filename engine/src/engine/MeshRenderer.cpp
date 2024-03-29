//
// Created by Jonathan Richard on 2024-02-12.
//

#include "engine/MeshRenderer.h"
#include "engine/graphics/MaterialResource.h"
#include "engine/graphics/MeshResource.h"
#include "engine/graphics/ShaderProgram.h"
#include "engine/graphics/VertexDataLayout.h"

void MeshRenderer::render(graphics::Renderer& renderer, const std::shared_ptr<MeshResource>& mesh, const std::shared_ptr<MaterialResource>& material, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
{
    struct UniformBufferObject {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    } ubo(model, view, projection);

    struct Constants {
        glm::vec3 cameraPos;
        glm::vec3 cameraDirection;
        glm::vec3 lightDir;
        float shininess = 32.0f;
    } constants(cameraPosition, cameraDirection, glm::vec3(-0.7f, -0.5f, -0.5f), 320.0f);

    material->setUniformBuffer("ubo", &ubo, sizeof(ubo), 0);
    material->setUniformBuffer("constants", &constants, sizeof(constants), 1);
//    material->setCullMode(CullMode::None);
//    material->setBlendMode(graphics::BlendMode::Opaque());
//    material->setDepthTestConfig(graphics::DepthTestConfig::Enable);

//    graphics::VertexDataLayout attribLayout({
//            { "inPosition", 0, VertexAttributeFormat::Float3 },
//            { "inNormal", 1, VertexAttributeFormat::Float3 },
//            { "inTexCoords", 2, VertexAttributeFormat::Float2 }
//    });
//
//    static auto vertexData = renderer.getDeviceManager().createIndexedVertexData(attribLayout, IndexFormat::UInt32, mesh.vertices.size(), mesh.indices.size());
//    vertexData->allocateVertexBuffer(renderer.getDevice(), mesh.vertices.size());
//    vertexData->allocateIndexBuffer(renderer.getDevice(), mesh.indices.size());
//
//    vertexData->pushVertices(mesh.vertices);
//    vertexData->pushIndices(mesh.indices);

    material->use(renderer);

    graphics::Renderable meshRenderable(material->getMaterial(), mesh->getVertexData());

    renderer.draw(meshRenderable);
}
