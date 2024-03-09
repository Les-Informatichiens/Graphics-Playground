//
// Created by Jonathan Richard on 2024-03-04.
//

#include "engine/Scene.h"
#include "engine/EntityView.h"
#include "engine/SceneNode.h"
#include "engine/SceneRenderData.h"
#include "engine/components/CameraComponent.h"
#include "engine/components/MeshComponent.h"


Scene::Scene()
    : registry(), entityMap()
{
    registry.on_construct<SceneNode>().connect<&Scene::linkSceneNodeWithEntity>();
    registry.on_update<SceneNode>().connect<&Scene::linkSceneNodeWithEntity>();
}

Scene::~Scene() = default;

void Scene::update(float dt)
{
    registry.view<SceneNode>().each([dt](SceneNode& node){
        if (!node.hasParent())
        {
            node.update(dt);
        }
    });
}

void Scene::getSceneRenderData(SceneRenderData& sceneRenderData) const
{
    sceneRenderData.reset();

    // iterate over all entities with a SceneNode and MeshComponent
    registry.view<SceneNode, MeshComponent>().each([&sceneRenderData](const SceneNode& node, const MeshComponent& mesh){
        if (!node.isVisible())
            return;

        sceneRenderData.meshRenderData.push_back({node.getWorldTransform().getModel(), mesh.getMesh().get(), mesh.getMaterial()});

        if (node.showBoundingBox)
        {
            auto model = node.getWorldTransform().getModel();
            Bounds bounds = mesh.getMesh()->bounds;

            // create line segments for each edge of the bounding box, we can join the segments from the top and bottom of the box
            glm::vec3 min = bounds.getMin();
            glm::vec3 max = bounds.getMax();
            glm::vec3 size = bounds.getSize();

            std::vector<glm::vec3> points = {
                    min,
                    min + glm::vec3(size.x, 0.0f, 0.0f),
                    min + glm::vec3(size.x, size.y, 0.0f),
                    min + glm::vec3(0.0f, size.y, 0.0f),
                    min,
                    min + glm::vec3(0.0f, 0.0f, size.z),
                    min + glm::vec3(size.x, 0.0f, size.z),
                    min + glm::vec3(size.x, size.y, size.z),
                    min + glm::vec3(0.0f, size.y, size.z),
                    min + glm::vec3(0.0f, 0.0f, size.z),
                    min + glm::vec3(size.x, 0.0f, size.z),
                    min + glm::vec3(size.x, 0.0f, 0.0f),
                    min + glm::vec3(size.x, size.y, 0.0f),
                    min + glm::vec3(size.x, size.y, size.z),
                    min + glm::vec3(0.0f, size.y, size.z),
                    min + glm::vec3(0.0f, size.y, 0.0f)};

            // split the points into lines
            for (size_t i = 0; i < points.size() - 1; i++)
            {
                sceneRenderData.lineRenderData.push_back({{model * glm::vec4(points[i], 1.0f), model * glm::vec4(points[i + 1], 1.0f)}, {1.0f, 0.0f, 0.0f, 1.0f}});
            }
        }
    });
}

void Scene::linkSceneNodeWithEntity(entt::registry& reg, entt::entity e)
{
    reg.get<SceneNode>(e).entity = {reg, e};
}

EntityView Scene::createEntity(const std::string& name)
{
    util::UUID uuid = util::UUID::generate();

    entityMap.emplace(uuid, EntityData{ name, uuid, registry.create() });

    EntityView entity = {uuid, this};
    entity.addComponent<SceneNode>(EntityView{uuid, this});

    return entity;
}

void Scene::destroyEntity(const EntityView& entity)
{
    registry.destroy(entity.getHandle());
    entityMap.erase(entity.getUUID());
}

void Scene::destroyEntity(util::UUID uuid)
{
    auto it = entityMap.find(uuid);
    if (it != entityMap.end())
    {
        registry.destroy(it->second.entity);
        entityMap.erase(it);
    }
}

std::optional<EntityView> Scene::getEntity(util::UUID uuid)
{
    auto it = entityMap.find(uuid);
    if (it != entityMap.end())
    {
        return { { it->second.uuid, this } };
    }
    return std::nullopt;
}

std::optional<EntityView> Scene::getEntityByName(const std::string& name)
{
    for (auto& [uuid, entityData]: entityMap)
    {
        if (entityData.name == name)
        {
            return { { uuid, this } };
        }
    }
    return std::nullopt;
}

entt::entity Scene::getEntityHandle(util::UUID uuid)
{
    auto it = entityMap.find(uuid);
    if (it != entityMap.end())
    {
        return it->second.entity;
    }
    return entt::null;
}

std::string& Scene::getEntityName(util::UUID uuid)
{
    auto it = entityMap.find(uuid);
    if (it != entityMap.end())
    {
        return it->second.name;
    }
    throw std::runtime_error("Entity not found");
}

std::vector<SceneNode*> Scene::getCameraNodes()
{
    std::vector<SceneNode*> cameraNodes;
    registry.view<SceneNode, CameraComponent>().each([&cameraNodes](SceneNode& node, CameraComponent& camera){
        cameraNodes.push_back(&node);
    });
    return cameraNodes;
}

std::vector<EntityView> Scene::getRootEntities()
{
    std::vector<EntityView> rootEntities;
    registry.view<SceneNode>().each([&rootEntities, this](entt::entity e, SceneNode& node){
        if (!node.hasParent())
        {
            rootEntities.push_back(node.getEntityView());
        }
    });
    return rootEntities;
}
