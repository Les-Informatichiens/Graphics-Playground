//
// Created by Jonathan Richard on 2024-02-13.
//

#include "engine/SceneNode.h"
#include "engine/graphics/Renderer.h"
#include <iostream>

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
        worldTransform = transform * parent->worldTransform;
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

SceneNode* SceneNode::findNode(const std::string& name)
{
    if (getName() == name)
    {
        return this;
    }
    for (auto& child: children)
    {
        auto node = child->findNode(name);
        if (node)
        {
            return node;
        }
    }
    return nullptr;
}

void SceneNode::visit(SceneNode::VisitorCallback visitor)
{
    visitor(*this);
    for (auto& child: children)
    {
        child->visit(visitor);
    }
}

void SceneNode::visitAndLeave(SceneNode::VisitorCallback callback, SceneNode::VisitorCallback leaveCallback)
{
    callback(*this);
    for (auto& child: children)
    {
        child->visitAndLeave(callback, leaveCallback);
    }
    leaveCallback(*this);
}

std::string SceneNode::getName() const
{
    return this->ownEntityView.getName();
}
