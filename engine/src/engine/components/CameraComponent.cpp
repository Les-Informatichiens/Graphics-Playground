//
// Created by Jonathan Richard on 2024-03-05.
//

#include "engine/components/CameraComponent.h"

CameraComponent::CameraComponent(std::shared_ptr<Camera> camera)
    : CameraComponent(camera, graphics::RenderTarget())
{
}

CameraComponent::CameraComponent(std::shared_ptr<Camera> camera, const graphics::RenderTarget& renderTarget)
    : camera(camera), renderTarget(renderTarget)
{
}

CameraComponent::~CameraComponent()
{
}

void CameraComponent::setRenderTarget(const graphics::RenderTarget& renderTarget)
{
    this->renderTarget = renderTarget;
}


const graphics::RenderTarget& CameraComponent::getRenderTarget() const
{
    return renderTarget;
}

std::shared_ptr<Camera> CameraComponent::getCamera() const
{
    return camera;
}

void CameraComponent::setCamera(std::shared_ptr<Camera> camera_)
{
    this->camera = camera_;
}
