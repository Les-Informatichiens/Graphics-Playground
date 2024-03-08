//
// Created by Jonathan Richard on 2024-03-04.
//

#include "engine/Scene.h"
#include "engine/EntityView.h"
#include "engine/SceneNode.h"
#include "engine/SceneRenderData.h"
#include "engine/components/CameraComponent.h"
#include "engine/components/MeshComponent.h"
#include "glm/gtx/string_cast.hpp"


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
        sceneRenderData.meshRenderData.push_back({node.getWorldTransform().getModel(), mesh.getMesh().get(), mesh.getMaterial()});
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
    auto& a = entity.addComponent<SceneNode>(EntityView{uuid, this});
    std::cout << glm::to_string(a.getTransform().getModel()) << std::endl;


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