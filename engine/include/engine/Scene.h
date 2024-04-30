//
// Created by Jonathan Richard on 2024-03-04.
//

#pragma once

#include "util/UUID.h"

#include "engine/graphics/TextureResource.h"
#include "entt/entt.hpp"

#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

struct SceneRenderData;
class EntityView;
class SceneNode;

class Scene
{
    friend class EntityView;
public:
    Scene();
    ~Scene();

    void update(float dt);

    EntityView createEntity(const std::string& name);

    void destroyEntity(const EntityView& entity);
    void destroyEntity(util::UUID uuid);

    std::optional<EntityView> getEntity(util::UUID uuid);
    std::optional<EntityView> getEntityByName(const std::string& name);

    std::vector<EntityView> getRootEntities();

    // retrieve all camera entities
    std::vector<SceneNode*> getCameraNodes();

    void getSceneRenderData(SceneRenderData& sceneRenderData) const;

    void setSkyboxTexture(std::shared_ptr<TextureResource> texture) { skyboxTexture = texture; }
    std::shared_ptr<TextureResource> getSkyboxTexture() const { return skyboxTexture; }

private:
    static void linkSceneNodeWithEntity(entt::registry &reg, entt::entity e);

    std::string& getEntityName(util::UUID uuid);
    entt::entity getEntityHandle(util::UUID uuid);

private:
    struct EntityData
    {
        std::string name;
        util::UUID uuid;
        entt::entity entity;
    };

    entt::registry registry;
    std::unordered_map<util::UUID, EntityData> entityMap;

    std::shared_ptr<TextureResource> skyboxTexture;
};
