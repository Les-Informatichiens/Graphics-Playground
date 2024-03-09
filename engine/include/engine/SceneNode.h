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
    friend class SceneEditor;
public:
    using VisitorCallback = std::function<void(SceneNode&)>;

    explicit SceneNode(const EntityView& entityView);

    void addChild(SceneNode* child);
    [[nodiscard]] SceneNode* getParent() const { return parent; }
    [[nodiscard]] bool hasParent() const { return parent != nullptr; }

    [[nodiscard]] std::string getName() const;

    SceneNode* findNode(const std::string& name);
    void visit(VisitorCallback callback);

    void visitAndLeave(VisitorCallback callback, VisitorCallback leaveCallback);

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

    template <typename T>
    void visitAndLeave(VisitorCallback callback, VisitorCallback leaveCallback)
    {
        if (ownEntityView.hasComponent<T>())
        {
            callback(*this);
        }
        for (auto* child : children)
        {
            child->visitAndLeave<T>(callback, leaveCallback);
        }
        leaveCallback(*this);
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

    void setShowBoundingBox(bool show) { showBoundingBox = show; }
    [[nodiscard]] bool getShowBoundingBox() const { return showBoundingBox; }

    void setVisible(bool visible_) { visible = visible_; }
    [[nodiscard]] bool isVisible() const { return visible; }

private:
    Transform transform;
    Transform worldTransform;
    entt::handle entity;
    EntityView ownEntityView;

    SceneNode* parent = nullptr;
    std::vector<SceneNode*> children;
    bool showBoundingBox = false;
    bool visible = true;
};

// Ensure components are not relocated in memory. This allows us to use raw
// pointers for them.
template <>
struct entt::component_traits<SceneNode> {
    using type = SceneNode;
    static constexpr bool in_place_delete = true;
    static constexpr std::size_t page_size = entt::internal::page_size<SceneNode>::value;
};
