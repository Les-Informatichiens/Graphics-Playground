//
// Created by Jonathan Richard on 2024-02-13.
//

#include "engine/graphics/Renderer.h"
#include <iostream>
#include "engine/SceneNode.h"

SceneNode::SceneNode(const EntityView& entityView)
    : ownEntityView(std::move(entityView)), transform(), worldTransform(), parent(nullptr), children()
{
}

void SceneNode::addChild(SceneNode* child)
{
    child->parent = this;
    children.push_back(child);
}

void SceneNode::update(float dt)
{
    if (parent)
    {
        worldTransform = parent->worldTransform * transform;
    }
    else
    {
        worldTransform = transform;
    }
    for (auto& child: children)
    {
//        std::cout << child->name << std::endl;
        child->update(dt);
    }
//    std::cout << "DONE UPD" << std::endl;
}

void SceneNode::draw(graphics::Renderer& renderer) const
{
    // Maybe we can have custom rendering operations done here
    // as of now we only render plain meshes.
    // I prefer to keep the rendering logic in the MeshRenderer
}

SceneNode* SceneNode::findNode(const std::string& name)
{
    if (getName() == name)
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

std::string SceneNode::getName() const
{
    return this->ownEntityView.getName();
}
