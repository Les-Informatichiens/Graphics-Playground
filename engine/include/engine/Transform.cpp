//
// Created by Jonathan Richard on 2024-02-13.
//

#include "Transform.h"
#include "glm/ext/matrix_transform.hpp"

Transform::Transform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale)
    : position_(position), rotation_(rotation), scale_(scale)
{
}

void Transform::translate(const glm::vec3& translation)
{
    this->position_ += translation;
}

void Transform::rotate(const glm::vec3& rotation)
{
    this->rotation_ += rotation;
}

void Transform::scale(const glm::vec3& scale)
{
    this->scale_ += scale;
}

void Transform::setPosition(const glm::vec3& position)
{
    this->position_ = position;
}

void Transform::setRotation(const glm::vec3& rotation)
{
    this->rotation_ = rotation;
}

void Transform::setScale(const glm::vec3& scale)
{
    this->scale_ = scale;
}

void Transform::reset()
{
    this->position_ = glm::vec3(0.0f);
    this->rotation_ = glm::vec3(0.0f);
    this->scale_ = glm::vec3(1.0f);
}

glm::mat4 Transform::getModel() const
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, this->position_);
    model = glm::rotate(model, glm::radians(this->rotation_.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(this->rotation_.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(this->rotation_.z), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, this->scale_);
    return model;
}

glm::mat4 Transform::getLookAt(const glm::vec3& target, const glm::vec3& up) const
{
    return glm::lookAt(this->position_, target, up);
}
