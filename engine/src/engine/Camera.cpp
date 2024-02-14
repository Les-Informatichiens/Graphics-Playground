//
// Created by Jonathan Richard on 2024-02-13.
//

#include "engine/Camera.h"
#include "glm/ext/matrix_clip_space.hpp"

Camera::Camera(const std::string& name)
    : Model(name)
{
    this->updateProjectionMatrix();
}

glm::mat4 Camera::getView()
{
    return glm::inverse(getTransform().getModel());
}

const glm::mat4& Camera::getProjection() const
{
    return this->projectionMatrix;
}

void Camera::updateProjectionMatrix()
{
    this->projectionMatrix = glm::perspective(glm::radians(this->fov), this->aspectRatio, this->nearClip, this->farClip);
}

void Camera::setProjectionConfig(float fov, float aspectRatio, float nearClip, float farClip)
{
    this->fov = fov;
    this->aspectRatio = aspectRatio;
    this->nearClip = nearClip;
    this->farClip = farClip;
    this->updateProjectionMatrix();
}