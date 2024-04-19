#pragma once

#include "i_material.h"
#include "textures/i_texture.h"

class glass final :
	public i_material
{
public:
	glass(const i_texture* p_texture, const float p_refractive_index) : albedo(p_texture),
	                                                                    refractive_index(p_refractive_index)
	{
	}

	bool alter_ray_direction(const ray& incident_ray, const vec3& normal, vec3& next_direction) const override
	{
		vec3 glass_normal = normal;
		float cos_theta = dot(glass_normal, incident_ray.get_direction());
		float etat = refractive_index;
		float etai = 1.0f; //refractive index of air

		//outside
		if (cos_theta < 0)
		{
			cos_theta = -cos_theta;
		}
		//inside
		else
		{
			glass_normal = -normal;
			std::swap(etai, etat);
		}
		const float eta = etai / etat;

		const vec3 glass_direction = normalize(incident_ray.get_direction());
		const float sin_theta = sqrt(etai - cos_theta * cos_theta);

		const bool refracts = eta * sin_theta > etai;

		if (refracts)
			next_direction = reflect(glass_direction, glass_normal);
		else
			next_direction = refract(glass_direction, glass_normal, eta);
		next_direction = normalize(next_direction);
		return true;
	}

	color3 color_at(const point3& t, const glm::vec2& uv) const override
	{
		return albedo->color_at(t, uv);
	}

private:
	const i_texture* albedo;
	const float refractive_index;
};
