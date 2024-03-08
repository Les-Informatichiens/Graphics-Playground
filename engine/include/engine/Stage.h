//
// Created by Jonathan Richard on 2024-03-04.
//

#pragma once

#include "Scene.h"
#include <memory>

class Stage
{
public:
    Stage() = default;
    ~Stage() = default;

    void update(float dt);
    Scene* getScene();
    void setScene(std::shared_ptr<Scene> newScene);

private:
    std::shared_ptr<Scene> scene;
};
