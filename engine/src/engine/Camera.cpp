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

glm::mat4 Camera::getView() const
{
    return glm::mat4(1.0);//glm::inverse(getTransform().getModel());
}

const glm::mat4& Camera::getProjection() const
{
    return this->projectionMatrix;
}

void Camera::updateProjectionMatrix()
{
    // Check if aspect ratio is NaN or infinite
    if (std::isnan(this->aspectRatio) || std::isinf(this->aspectRatio)) {
        return;
    }

    if (this->projectionType == ProjectionType::Orthographic)
    {
        this->projectionMatrix = glm::ortho(-this->orthoSize_, this->orthoSize_, -this->orthoSize_ / this->aspectRatio, this->orthoSize_ / this->aspectRatio, this->nearClip, this->farClip);
        return;
    }
    else if (this->projectionType == ProjectionType::Perspective)
    {
        this->projectionMatrix = glm::perspective(glm::radians(this->fov), this->aspectRatio, this->nearClip, this->farClip);
        return;
    }
}

void Camera::setProjectionConfig(float fov, float aspectRatio, float nearClip, float farClip)
{
    this->fov = fov;
    this->aspectRatio = aspectRatio;
    this->nearClip = nearClip;
    this->farClip = farClip;
    this->viewportWidth = static_cast<int>(2.0f * this->farClip * glm::tan(glm::radians(this->fov / 2.0f)));
    this->viewportHeight = static_cast<int>(2.0f * this->farClip * glm::tan(glm::radians(this->fov / 2.0f)));
    this->updateProjectionMatrix();
}
int Camera::getViewportWidth() const
{
    return this->viewportWidth;
}

int Camera::getViewportHeight() const
{
    return this->viewportHeight;
}

void Camera::setProjectionType(Camera::ProjectionType type)
{
    this->projectionType = type;
    this->updateProjectionMatrix();
}

void Camera::setProjectionConfigVP(float fov, float viewportWidth, float viewportHeight, float nearClip, float farClip)
{
    this->viewportWidth = static_cast<int>(viewportWidth);
    this->viewportHeight = static_cast<int>(viewportHeight);
    this->aspectRatio = viewportWidth / viewportHeight;
    this->nearClip = nearClip;
    this->farClip = farClip;
    this->updateProjectionMatrix();
}

util::Ray Camera::screenPointToRay(const Transform& transform, float x, float y, int screenWidth, int screenHeight) const
{
    auto view = glm::inverse(transform.getModel());
    glm::vec4 viewport = {0.0f, 0.0f, static_cast<float>(screenWidth), static_cast<float>(screenHeight)};
    glm::vec3 screenPos = {x, screenHeight - y, 0.0f};
    glm::vec3 near = glm::unProject(screenPos, view, this->projectionMatrix, viewport);
    screenPos.z = 1.0f;
    glm::vec3 far = glm::unProject(screenPos, view, this->projectionMatrix, viewport);
    return util::Ray(near, glm::normalize(far - near));
}
