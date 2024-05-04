//
// Created by Jonathan Richard on 2024-03-08.
//

#pragma once

#include "engine/util/Ray.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
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

    bool intersects(const glm::vec3& point) const
    {
        return point.x >= center.x - extents.x && point.x <= center.x + extents.x &&
               point.y >= center.y - extents.y && point.y <= center.y + extents.y &&
               point.z >= center.z - extents.z && point.z <= center.z + extents.z;
    }

    bool intersects(util::Ray ray) const
    {
        glm::vec3 invDir = 1.0f / ray.getDirection();
        glm::vec3 t0 = (getMin() - ray.getOrigin()) * invDir;
        glm::vec3 t1 = (getMax() - ray.getOrigin()) * invDir;
        glm::vec3 tmin = glm::min(t0, t1);
        glm::vec3 tmax = glm::max(t0, t1);
        float tminMax = glm::max(tmin.x, glm::max(tmin.y, tmin.z));
        float tmaxMin = glm::min(tmax.x, glm::min(tmax.y, tmax.z));
        return tminMax <= tmaxMin;
    }

    glm::vec3 getIntersectionPoint(util::Ray ray) const
    {
        glm::vec3 invDir = 1.0f / ray.getDirection();
        glm::vec3 t0 = (getMin() - ray.getOrigin()) * invDir;
        glm::vec3 t1 = (getMax() - ray.getOrigin()) * invDir;
        glm::vec3 tmin = glm::min(t0, t1);
        glm::vec3 tmax = glm::max(t0, t1);
        float tminMax = glm::max(tmin.x, glm::max(tmin.y, tmin.z));
        float tmaxMin = glm::min(tmax.x, glm::min(tmax.y, tmax.z));
        if (tminMax <= tmaxMin)
        {
            return ray.getOrigin() + ray.getDirection() * tminMax;
        }
        return glm::vec3(0.0f);
    }

    glm::vec3 getNormal(const glm::vec3& point) const
    {
        glm::vec3 min = getMin();
        glm::vec3 max = getMax();
        glm::vec3 normal = glm::vec3(0.0f);
        if (point.x <= min.x)
        {
            normal = glm::vec3(-1.0f, 0.0f, 0.0f);
        }
        else if (point.x >= max.x)
        {
            normal = glm::vec3(1.0f, 0.0f, 0.0f);
        }
        else if (point.y <= min.y)
        {
            normal = glm::vec3(0.0f, -1.0f, 0.0f);
        }
        else if (point.y >= max.y)
        {
            normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }
        else if (point.z <= min.z)
        {
            normal = glm::vec3(0.0f, 0.0f, -1.0f);
        }
        else if (point.z >= max.z)
        {
            normal = glm::vec3(0.0f, 0.0f, 1.0f);
        }
        return normal;
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
