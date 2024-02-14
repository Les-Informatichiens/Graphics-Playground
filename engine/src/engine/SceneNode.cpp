//
// Created by Jonathan Richard on 2024-02-13.
//

#include "engine/SceneNode.h"
#include "engine/MeshRenderer.h"
#include "engine/graphics/Renderer.h"
#include <iostream>

SceneNode::SceneNode(std::string name)
    : name(std::move(name)), transform(), worldTransform(), parent(nullptr), children(), mesh(nullptr)
{
}

void SceneNode::addChild(std::unique_ptr<SceneNode> child)
{
    child->parent = this;
    children.push_back(std::move(child));
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
        std::cout << child->name << std::endl;
        child->update(dt);
    }
    std::cout << "DONE UPD" << std::endl;
}
void SceneNode::draw(graphics::Renderer& renderer) const
{
    std::cout << "RENDERING " << this->name << std::endl;

    if (mesh)
    {
        MeshRenderer meshRenderer;
        meshRenderer.render(renderer, *mesh, worldTransform, renderer.getCamera());
    }
    std::cout << "DONE RENDERING " << this->name << std::endl;
}
