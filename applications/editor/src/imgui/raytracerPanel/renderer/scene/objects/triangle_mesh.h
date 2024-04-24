#pragma once
#include <cstdint>
#include <utility>
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
	triangle_mesh(const i_material* material, const int nb_triangles, std::vector<std::vector<Vertex>> p_triangles)
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
            const Vertex& v0 = triangles[i][0];
            const Vertex& v1 = triangles[i][1];
            const Vertex& v2 = triangles[i][2];
            glm::vec2 hit_uv{0, 0};
            if (intersectRayTriangle(acne_corrected_origin, direction, v0.position, v1.position, v2.position, hit_uv, distance) &&
                distance > 0.001f && min_distance > distance)
            {
                min_distance = distance;
                t = p_ray.move(min_distance);
                // Interpolate the normals based on the uv coordinates
                normal = glm::normalize((1 - hit_uv.x - hit_uv.y) * v0.normal + hit_uv.x * v1.normal + hit_uv.y * v2.normal);
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

    [[nodiscard]] color3 color_at(const point3& t, const glm::vec2& uv) const override
    {
        return material->color_at(t, uv);
    }

    float get_shininess() const override
    {
        return material->get_shininess();
    }

private:
	const i_material* material;
	const uint32_t nb_triangles{};
    std::vector<std::vector<Vertex>> triangles;
};
