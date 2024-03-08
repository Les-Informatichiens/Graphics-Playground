//
// Created by Jonathan Richard on 2024-03-04.
//

#pragma once

#include "Scene.h"
#include "entt/entt.hpp"
#include "util/UUID.h"

class Scene;
class SceneNode;

class EntityView
{
    friend class Scene;
public:
    EntityView(const util::UUID& uuid, Scene* scene);
    ~EntityView();

    EntityView(const EntityView& other);
    EntityView& operator=(const EntityView& other);

    template<typename T, typename... Args>
    T& addComponent(Args&&... args)
    {
        return scene->registry.emplace<T>(entityHandle, std::forward<Args>(args)...);
    }

    template<typename T>
    T& getComponent()
    {
        return scene->registry.get<T>(entityHandle);
    }

    template<typename T>
    bool hasComponent()
    {
        return scene->registry.any_of<T>(entityHandle);
    }

    template<typename T>
    void removeComponent()
    {
        scene->registry.remove<T>(entityHandle);
    }

    [[nodiscard]] SceneNode& getSceneNode();

    [[nodiscard]] const util::UUID& getUUID() const
    {
        return uuid;
    }

    [[nodiscard]] const std::string& getName() const
    {
        return scene->getEntityName(uuid);
    }

private:
    // For the case where the entity is created from the scene, internal use only
    EntityView(const util::UUID& uuid, entt::entity entityHandle, Scene* scene);

    [[nodiscard]] entt::entity getHandle() const;

private:
//    class Impl;
//    std::unique_ptr<Impl> pimpl;
    util::UUID uuid;
    entt::entity entityHandle;
    Scene* scene = nullptr;
    SceneNode* sceneNode = nullptr;
};