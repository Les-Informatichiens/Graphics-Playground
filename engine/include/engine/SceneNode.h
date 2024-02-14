//
// Created by Jonathan Richard on 2024-02-13.
//

#pragma once

#include "engine/Mesh.h"
#include "engine/Transform.h"
#include <memory>
#include <vector>

namespace graphics {
class Renderer;
}
class SceneNode
{
public:
    explicit SceneNode(std::string name);

    void addChild(std::unique_ptr<SceneNode> child);

    [[nodiscard]] std::string getName() const { return name; }

    [[nodiscard]] std::shared_ptr<Mesh> getMesh() const { return mesh; }
    void setMesh(std::shared_ptr<Mesh> mesh_) { this->mesh = mesh_; }

    SceneNode* findNode(const std::string& name)
    {
        if (this->name == name)
        {
            return this;
        }
        for (auto& child : children)
        {
            auto node = child->findNode(name);
            if (node)
            {
                return node;
            }
        }
        return nullptr;
    }

    void setTransform(const Transform& transform_) { transform = transform_; }
    Transform& getTransform() { return transform; }
    [[nodiscard]] const Transform& getTransform() const { return transform; }

    [[nodiscard]] const Transform& getWorldTransform() const { return worldTransform; }

    virtual void update(float dt);
    virtual void draw(graphics::Renderer& renderer) const;

    std::vector<std::unique_ptr<SceneNode>>::const_iterator begin() const { return children.begin(); }
    std::vector<std::unique_ptr<SceneNode>>::const_iterator end() const { return children.end(); }
    std::vector<std::unique_ptr<SceneNode>>& getChildren() { return children; }

private:
    std::string name;

    std::shared_ptr<Mesh> mesh;
    Transform transform;
    Transform worldTransform;

    SceneNode* parent;
    std::vector<std::unique_ptr<SceneNode>> children;
};
