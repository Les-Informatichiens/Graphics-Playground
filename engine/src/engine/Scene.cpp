//
// Created by Jonathan Richard on 2024-03-04.
//

#include "engine/Scene.h"
#include "engine/EntityView.h"
#include "engine/SceneRenderData.h"
#include "engine/SceneNode.h"


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
    sceneRenderData.meshRenderData.clear();
    registry.view<SceneNode>().each([&sceneRenderData](const SceneNode& node)
    {
        if (auto mesh = node.getMesh())
        {
            sceneRenderData.meshRenderData.push_back({node.getWorldTransform().getModel(), mesh.get()});
        }
    });
//    for (const auto& node: nodes)
//    {
//        if (auto mesh = node->getMesh())
//        {
//            sceneRenderData.meshRenderData.push_back({node->getTransform().getModel(), mesh.get()});
//        }
//    }
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
