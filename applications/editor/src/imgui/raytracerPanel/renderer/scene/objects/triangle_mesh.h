#pragma once
#include <cstdint>
#include <vector>

#include "i_object.h"
#include "../../ray.h"
#include "../../utils.h"
#include "glm/geometric.hpp"
#include "glm/gtx/intersect.hpp"
#include "glm/gtx/normal.hpp"
#include "materials/i_material.h"

class triangle_mesh final : public i_object
{
public:
	triangle_mesh(const i_material* material, const int nb_triangles, std::vector<std::vector<point3>> p_triangles)
		: material(material),
		  nb_triangles(nb_triangles),
		  triangles(std::move(p_triangles))
	{
	}

    bool intersect(const ray& p_ray, point3& t, vec3& normal, glm::vec2& uv) const override
    {
        const point3 acne_corrected_origin = p_ray.move(0.01f);
        const vec3 direction = normalize(p_ray.get_direction());
        bool intersected{false};
        float min_distance{std::numeric_limits<float>::max()};
        float distance{0};

        for (uint32_t i = 0; i < nb_triangles; ++i)
        {
            const point3 v0 = triangles[i][0];
            const point3 v1 = triangles[i][1];
            const point3 v2 = triangles[i][2];

            glm::vec2 hit_uv{0, 0};
            if (intersectRayTriangle(acne_corrected_origin, direction, v0, v1, v2, hit_uv, distance) &&
                distance > 0.001f && min_distance > distance)
            {
                min_distance = distance;
                t = p_ray.move(min_distance);
                normal = triangleNormal(v0, v1, v2);
                intersected = true;
                uv = hit_uv;
            }
        }

        return intersected;
    }

	bool alter_ray_direction(const ray& incident_ray, const vec3& normal, vec3& next_direction) const override
	{
		return material->alter_ray_direction(incident_ray, normal, next_direction);
	}

	color3 color_at(const point3& t, const glm::vec2& uv) const override
	{
		return material->color_at(t, uv);
	}

private:
	const i_material* material;
	const uint32_t nb_triangles{};
	const std::vector<std::vector<point3>> triangles;
};
