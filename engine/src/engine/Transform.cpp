//
// Created by Jonathan Richard on 2024-02-13.
//

#include "engine/Transform.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtx/matrix_decompose.hpp"
#include <iostream>

Transform::Transform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale)
    : position_(position), rotation_(rotation), scale_(scale)
{
}

Transform::Transform(const glm::mat4& modelMatrix)
{
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(modelMatrix, this->scale_, this->rotation_, this->position_, skew, perspective);
}

void Transform::translate(const glm::vec3& translation)
{
    this->position_ += translation;
}

void Transform::rotate(const glm::vec3& rotation)
{
    this->rotation_ = glm::normalize(glm::quat(rotation) * this->rotation_);
}

void Transform::rotate(const glm::quat& rotation)
{
    this->rotation_ = glm::normalize(rotation * this->rotation_);
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
    this->rotation_ = glm::normalize(glm::quat(rotation));
}

void Transform::setRotation(const glm::quat& rotation)
{
    this->rotation_ = glm::normalize(rotation);
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
    model = model * glm::mat4_cast(this->rotation_);
    model = glm::scale(model, this->scale_);
    return model;
}
glm::mat4 Transform::getLookAt(const glm::vec3& target, const glm::vec3& up) const
{
    return glm::lookAt(this->position_, target, up);
}

Transform Transform::operator*(const Transform& other) const
{
    // create a new transform that is the result of the multiplication
    // translation must take account the scale and rotation of the other transform
    Transform result;

    // Rotate the position vector of the other transform by the rotation of the current transform
    glm::vec3 rotatedPosition = glm::rotate(other.rotation_, this->position_);

    // Calculate the resulting position considering the scale and rotation of both transforms
    result.position_ = other.position_ + rotatedPosition;

    // Combine the rotations of both transforms
    result.rotation_ = other.rotation_ * this->rotation_;

    // Combine the scales of both transforms
    result.scale_ = this->scale_ * other.scale_;

    return result;
}

void Transform::lookAt(const glm::vec3& target, const glm::vec3& up)
{
    glm::vec3 direction = glm::normalize(target - this->position_);
    this->rotation_ = glm::quatLookAt(direction, up);
}

Transform& Transform::operator=(const Transform& other)
{
    this->position_ = other.position_;
    this->rotation_ = other.rotation_;
    this->scale_ = other.scale_;
    return *this;
}
