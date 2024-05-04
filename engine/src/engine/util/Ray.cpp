//
// Created by Jonathan Richard on 2024-05-02.
//

#include "engine/util/Ray.h"

namespace util {

Ray::Ray()
{
}
Ray::Ray(const glm::vec3& origin, const glm::vec3& direction)
    : origin(origin), direction(direction)
{
}

const glm::vec3& Ray::getOrigin() const
{
    return origin;
}

const glm::vec3& Ray::getDirection() const
{
    return direction;
}

glm::vec3 Ray::getPoint(float t) const
{
    return origin + direction * t;
}

}