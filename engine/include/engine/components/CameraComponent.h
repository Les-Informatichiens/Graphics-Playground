//
// Created by Jonathan Richard on 2024-03-05.
//

#pragma once


#include "engine/Camera.h"
#include "engine/graphics/RenderTarget.h"

#include <memory>

class CameraComponent
{
public:
    CameraComponent(std::shared_ptr<Camera> camera);
    CameraComponent(std::shared_ptr<Camera> camera, const graphics::RenderTarget& renderTarget);
    ~CameraComponent();

    void setRenderTarget(const graphics::RenderTarget& renderTarget);
    const graphics::RenderTarget& getRenderTarget() const;

    void setCamera(std::shared_ptr<Camera> camera_);
    std::shared_ptr<Camera> getCamera() const;

private:
    std::shared_ptr<Camera> camera;
    graphics::RenderTarget renderTarget;
};
