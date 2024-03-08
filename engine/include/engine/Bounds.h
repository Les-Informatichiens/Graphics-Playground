//
// Created by Jonathan Richard on 2024-03-08.
//

#pragma once

#include "glm/glm.hpp"
#include <vector>

class Bounds
{
public:
    Bounds(glm::vec3 center, glm::vec3 size) : center(center), extents(size / 2.0f) {}

    ~Bounds() = default;

    bool contains(const glm::vec3& point) const
    {
        return point.x >= center.x - extents.x && point.x <= center.x + extents.x &&
               point.y >= center.y - extents.y && point.y <= center.y + extents.y &&
               point.z >= center.z - extents.z && point.z <= center.z + extents.z;
    }

    bool intersects(const Bounds& other) const
    {
        return center.x - extents.x <= other.center.x + other.extents.x &&
               center.x + extents.x >= other.center.x - other.extents.x &&
               center.y - extents.y <= other.center.y + other.extents.y &&
               center.y + extents.y >= other.center.y - other.extents.y &&
               center.z - extents.z <= other.center.z + other.extents.z &&
               center.z + extents.z >= other.center.z - other.extents.z;
    }

    [[nodiscard]] glm::vec3 getCenter() const { return center; }
    [[nodiscard]] glm::vec3 getExtents() const { return extents; }
    [[nodiscard]] glm::vec3 getSize() const { return extents * 2.0f; }
    [[nodiscard]] glm::vec3 getMin() const { return center - extents; }
    [[nodiscard]] glm::vec3 getMax() const { return center + extents; }

    [[nodiscard]] std::vector<glm::vec3> getCorners() const
    {
        std::vector<glm::vec3> corners(8);
        const glm::vec3 min = getMin();
        const glm::vec3 max = getMax();

        corners[0] = glm::vec3(min.x, min.y, min.z);
        corners[1] = glm::vec3(max.x, min.y, min.z);
        corners[2] = glm::vec3(min.x, max.y, min.z);
        corners[3] = glm::vec3(max.x, max.y, min.z);
        corners[4] = glm::vec3(min.x, min.y, max.z);
        corners[5] = glm::vec3(max.x, min.y, max.z);
        corners[6] = glm::vec3(min.x, max.y, max.z);
        corners[7] = glm::vec3(max.x, max.y, max.z);

        return corners;
    }

    bool operator==(const Bounds& other) const
    {
        return center == other.center && extents == other.extents;
    }

    bool operator!=(const Bounds& other) const
    {
        return !(*this == other);
    }


private:
    glm::vec3 center;
    glm::vec3 extents;
    std::vector<glm::vec3> corners;
};
