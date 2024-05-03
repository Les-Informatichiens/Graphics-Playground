//
// Created by Jonathan Richard on 2024-05-02.
//

#pragma once

#include "glm/glm.hpp"

namespace util {

class Ray
{
public:
    Ray();
    Ray(const glm::vec3& origin, const glm::vec3& direction);

    const glm::vec3& getOrigin() const;
    const glm::vec3& getDirection() const;

    glm::vec3 getPoint(float t) const;

private:
    glm::vec3 origin;
    glm::vec3 direction;
};

}
