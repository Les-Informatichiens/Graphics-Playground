#pragma once
#include "i_object.h"
#include "glm/glm.hpp"
#include "materials/i_material.h"

class box final : public i_object
{
public:
    box(const point3& p_min, const point3& p_max, const i_material* p_material) 
        : min(p_min), max(p_max), material(p_material)
    {
    }

    bool box::intersect(const ray& p_ray, point3& t, vec3& normal, glm::vec2& uv) const {
        vec3 inv_dir = 1.0f / p_ray.get_direction();

        vec3 t_min = (min - p_ray.get_origin()) * inv_dir;
        vec3 t_max = (max - p_ray.get_origin()) * inv_dir;

        vec3 min_values = glm::min(t_min, t_max);
        vec3 max_values = glm::max(t_min, t_max);

        float t_near = glm::max(glm::max(min_values.x, min_values.y), min_values.z);
        float t_far = glm::min(glm::min(max_values.x, max_values.y), max_values.z);

        if (t_far < 0.0f || t_near > t_far) {
            return false;
        }

        t = p_ray.get_origin() + t_near * p_ray.get_direction();
        normal = glm::vec3(t.x > min.x, t.y > min.y, t.z > min.z) - glm::vec3(t.x < max.x, t.y < max.y, t.z < max.z);

        vec3 size = max - min;
        uv.x = (t.x - min.x) / size.x;
        uv.y = (t.y - min.y) / size.y;

        return true;
    }

    bool alter_ray_direction(const ray& incident_ray, const vec3& normal, vec3& next_direction) const override
    {
        return material->alter_ray_direction(incident_ray, normal, next_direction);
    }

    color3 color_at(const point3& t, const glm::vec2& uv) const override
    {
        return material->color_at(t, uv);
    }

    [[nodiscard]] float get_shininess() const override
    {
        return material->get_shininess();
    }

private:
    const point3 min{};
    const point3 max{};
    const i_material* material;
};