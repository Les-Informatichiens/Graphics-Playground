//
// Created by Jonathan Richard on 2024-03-04.
//

#include "engine/EntityView.h"
#include "engine/Scene.h"
#include "engine/SceneNode.h"

#include <memory>

EntityView::EntityView(const util::UUID& uuid, Scene* scene)
    : uuid(uuid), entityHandle(), scene(scene)
{
    if (scene == nullptr)
    {
        throw std::runtime_error("Scene is null");
    }
    this->entityHandle = scene->getEntityHandle(uuid);
    if (this->entityHandle == entt::null)
    {
        throw std::runtime_error("Entity not found");
    }
}

// For the case where the entity is created from the scene, internal use only
EntityView::EntityView(const util::UUID& uuid, entt::entity entityHandle, Scene* scene)
        : uuid(uuid), entityHandle(entityHandle), scene(scene)
{
}

EntityView::EntityView(const EntityView& other)
{
    this->uuid = other.uuid;
    this->entityHandle = other.entityHandle;
    this->scene = other.scene;
}

EntityView& EntityView::operator=(const EntityView& other)
{
    this->uuid = other.uuid;
    this->entityHandle = other.entityHandle;
    this->scene = other.scene;
    return *this;
}

EntityView::~EntityView() = default; // Define the destructor here to avoid including the Impl class in the header file

entt::entity EntityView::getHandle() const
{
    return this->entityHandle;
}

SceneNode& EntityView::getSceneNode()
{
    if (sceneNode == nullptr)
    {
        sceneNode = &getComponent<SceneNode>();
        if (sceneNode == nullptr)
        {
            throw std::runtime_error("Entity does not have SceneNode component");
        }
        return getComponent<SceneNode>();
    }
    return *sceneNode;
}
