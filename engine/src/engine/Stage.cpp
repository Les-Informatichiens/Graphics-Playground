//
// Created by Jonathan Richard on 2024-03-04.
//

#include "engine/Stage.h"

Scene* Stage::getScene()
{
    return scene.get();
}

void Stage::setScene(std::shared_ptr<Scene> newScene)
{
    this->scene = std::move(newScene);
}

void Stage::update(float dt)
{
    if (this->scene)
        this->scene->update(dt);
}
