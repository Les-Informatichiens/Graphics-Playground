//
// Created by Jonathan Richard on 2024-03-04.
//

#include "engine/EntityView.h"
#include "engine/Scene.h"

#include <memory>

//class Entity::Impl
//{
//public:
//    Impl(entt::entity handle, Scene* scene) : entityHandle(handle), scene(scene) {}
//
//    template<typename T, typename... Args>
//    T& addComponent(Args&&... args)
//    {
//        return scene->registry.emplace<T>(entityHandle, std::forward<Args>(args)...);
//    }
//
//    template<typename T>
//    T& getComponent()
//    {
//        return scene->registry.get<T>(entityHandle);
//    }
//
//    template<typename T>
//    bool hasComponent()
//    {
//        return scene->registry.any_of<T>(entityHandle);
//    }
//
//    template<typename T>
//    void removeComponent()
//    {
//        scene->registry.remove<T>(entityHandle);
//    }
//
//public:
//    entt::entity entityHandle;
//    Scene* scene;
//};
//
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
//pimpl(std::make_unique<Impl>(handle, scene)) {}
//
EntityView::~EntityView() = default; // Define the destructor here to avoid including the Impl class in the header file
//
//template<typename T, typename... Args>
//T& Entity::addComponent(Args&&... args)
//{
//    return pimpl->addComponent<T>(std::forward<Args>(args)...);
//}
//
//template<typename T>
//T& Entity::getComponent()
//{
//    return pimpl->getComponent<T>();
//}
//
//template<typename T>
//bool Entity::hasComponent()
//{
//    return pimpl->hasComponent<T>();
//}
//
//template<typename T>
//void Entity::removeComponent()
//{
//    pimpl->removeComponent<T>();
//}
//
entt::entity EntityView::getHandle() const
{
    return this->entityHandle;
}
