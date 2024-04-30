#pragma once

#include "i_material.h"
#include "textures/i_texture.h"

class glass final :
	public i_material
{
public:
    glass(const i_texture*
                  p_texture,
          const float
                  p_refractive_index,
          const float
                  shininess) : albedo(p_texture),
	                                                                    refractive_index(p_refractive_index), shininess(shininess)
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

    	// Compute Fresnel effect
    	float R0 = (etai - etat) / (etai + etat);
    	R0 *= R0;
    	float cosX = -dot(glass_normal, glass_direction);
    	if (etai > etat)
    	{
    		const float sinT2 = etat * (1.0f - cosX * cosX) / etai;
    		if (sinT2 > 1.0f) // Total internal reflection
    		{
    			next_direction = reflect(glass_direction, glass_normal) + 0.001f;
    			return true;
    		}
    		cosX = sqrt(1.0f - sinT2);
    	}
    	const float x = 1.0f - cosX;
    	const float R = R0 + (1.0f - R0) * x * x * x * x * x; // Schlick approximation

    	if (refracts && ((rand() / (RAND_MAX + 1.0)) >= R))
    	{
    		next_direction = refract(glass_direction, glass_normal, eta);
    	}
    	else
    	{
    		next_direction = reflect(glass_direction, glass_normal);
    	}

    	next_direction = normalize(next_direction);
    	return true;
    }

	[[nodiscard]] color3 color_at(const point3& t, const glm::vec2& uv) const override
	{
		return albedo->color_at(t, uv);
	}
    [[nodiscard]] float get_shininess() const override
    {
        return shininess;
    }
private:
	const i_texture* albedo;
	const float refractive_index;
    const float shininess;

};
