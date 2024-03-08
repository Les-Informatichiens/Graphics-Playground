//
// Created by Jonathan Richard on 2024-02-13.
//

#pragma once

#include "Model.h"
#include "Transform.h"

class Camera : public Model
{
public:
    enum class ProjectionType
    {
        Perspective,
        Orthographic
    };

    explicit Camera(const std::string& name);


    void setProjectionConfig(float fov, float aspectRatio, float nearClip, float farClip);

    [[nodiscard]] glm::mat4 getView();
    [[nodiscard]] const glm::mat4& getProjection() const;
    [[nodiscard]] int getViewportWidth() const;
    [[nodiscard]] int getViewportHeight() const;
private:
    void updateProjectionMatrix();

    float fov = 60.0f;
    float aspectRatio = 16.0f / 9.0f;
    float nearClip = 0.1f;
    float farClip = 1000.0f;
    int viewportWidth = 1920;
    int viewportHeight = 1080;

    ProjectionType projectionType = ProjectionType::Perspective;

    glm::mat4 projectionMatrix;
};
