#pragma once
#include "i_material.h"
#include "glm/gtc/random.hpp"

class metal final : public i_material
{
public:
	metal(const i_texture* p_texture, const float p_diffusion) : albedo(p_texture), diffusion(p_diffusion)
	{
	}

	bool alter_ray_direction(const ray& incident_ray, const vec3& normal, vec3& next_direction) const override
	{
		const vec3 reflection = reflect(incident_ray.get_direction(), normal);

		next_direction = reflection + diffusion * glm::sphericalRand(0.05f);

		next_direction = normalize(next_direction);
		if (glm::abs(next_direction.x) < 0.00001f && glm::abs(next_direction.y) < 0.00001f && glm::abs(next_direction.z)
			< 0.00001f)
		{
			next_direction = normal;
		}
		return true;
	}

	color3 color_at(const point3& t, const glm::vec2& uv) const override
	{
		return albedo->color_at(t, uv);
	}

private:
	const i_texture* albedo;
	const float diffusion;
};
