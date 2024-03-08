//
// Created by Jonathan Richard on 2024-03-04.
//

#pragma once

#include "util/UUID.h"

#include "entt/entt.hpp"

#include <vector>
#include <memory>
#include <unordered_map>
#include <optional>

struct SceneRenderData;
class EntityView;

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

    void getSceneRenderData(SceneRenderData& sceneRenderData) const;

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
};
