//
// Created by Jonathan Richard on 2024-02-13.
//

#pragma once

#include "EntityView.h"
#include "engine/Transform.h"
#include "entt/entt.hpp"

#include <memory>
#include <string>
#include <vector>

namespace graphics {
class Renderer;
}

class SceneNode
{
    friend class Scene;
public:
    using VisitorCallback = std::function<void(SceneNode&)>;

    explicit SceneNode(const EntityView& entityView);

    void addChild(SceneNode* child);
    [[nodiscard]] SceneNode* getParent() const { return parent; }
    [[nodiscard]] bool hasParent() const { return parent != nullptr; }

    [[nodiscard]] std::string getName() const;

    SceneNode* findNode(const std::string& name);
    void visit(VisitorCallback callback);

    template <typename T>
    void visit(VisitorCallback callback)
    {
        if (ownEntityView.hasComponent<T>())
        {
            callback(*this);
        }
        for (auto* child : children)
        {
            child->visit<T>(callback);
        }
    }

    void setTransform(const Transform& transform_) { transform = transform_; }
    Transform& getTransform() { return transform; }
    [[nodiscard]] const Transform& getTransform() const { return transform; }

    [[nodiscard]] const Transform& getWorldTransform() const { return worldTransform; }

    [[nodiscard]] EntityView getEntityView() const { return ownEntityView; }

    virtual void update(float dt);

    [[nodiscard]] std::vector<SceneNode*>::const_iterator begin() const { return children.begin(); }
    [[nodiscard]] std::vector<SceneNode*>::const_iterator end() const { return children.end(); }
    std::vector<SceneNode*>& getChildren() { return children; }

private:
    Transform transform;
    Transform worldTransform;
    entt::handle entity;
    EntityView ownEntityView;

    SceneNode* parent = nullptr;
    std::vector<SceneNode*> children;
};

// Ensure components are not relocated in memory. This allows us to use raw
// pointers for them.
template <>
struct entt::component_traits<SceneNode> {
    using type = SceneNode;
    static constexpr bool in_place_delete = true;
    static constexpr std::size_t page_size = entt::internal::page_size<SceneNode>::value;
};
