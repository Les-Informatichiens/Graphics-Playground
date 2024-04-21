#pragma once
#include <vector>

#include "../utils.h"
#include "glm/gtc/constants.hpp"
#include "objects/i_object.h"
#include "objects/lights/i_light.h"

class object_manager
{
public:
	object_manager() = default;

    ~object_manager()
	{
		for (auto& object : objects)
		{
			delete object;
			object = nullptr;
		}
	}

	void add_object(i_object* object)
	{
		objects.push_back(object);
	}

	void add_light(const i_light* light)
	{
		lights.push_back(light);
	}

	color3 compute_global_illumination(const ray& incident_ray, const uint32_t& max_rays,
	                                   const i_object* closest_hit_object,
	                                   point3& closest_hit_t, const vec3& closest_hit_normal,
	                                   const glm::vec2& closest_hit_uv,
	                                   const color3& local_color) const
	{
		color3 global_color{0, 0, 0};
		vec3 next_ray_direction{};
		if (closest_hit_object->alter_ray_direction(incident_ray, closest_hit_normal, next_ray_direction))
		{
			global_color = closest_hit_object->color_at(closest_hit_t, closest_hit_uv) * compute_color(
				{closest_hit_t, next_ray_direction}, max_rays - 1);

			return local_color + global_color;
		}
		return local_color;
	}

	void compute_local_illumination(const i_object* closest_hit_object, const point3& closest_hit_t,
	                                const vec3& closest_hit_normal, const glm::vec2& closest_hit_uv,
	                                color3& local_color) const
	{
		local_color = {0, 0, 0};
		for (const auto& light : lights)
		{
			vec3 direction_to_light{};
			light->direction_to(closest_hit_t, direction_to_light);
			ray shadow_ray{closest_hit_t, direction_to_light};
			bool hit_something{false};
			point3 t{0.0f, 0.0f, 0.0f};
			vec3 normal{0.0f, 0.0f, 0.0f};
			glm::vec2 uv;
			for (const auto& object : objects)
			{
				if (object->intersect(shadow_ray, t, normal, uv))
				{
					hit_something = true;
					break;
				}
			}
			if (!hit_something)
			{
				local_color += closest_hit_object->color_at(closest_hit_t, closest_hit_uv) / glm::pi<float>() * light->
					emit(
						closest_hit_t, closest_hit_normal);
			}
		}
	}

	void compute_closest_intersection(const ray& incident_ray,
	                                  i_object*& closest_hit_object, point3& closest_hit_t, vec3& closest_hit_normal,
	                                  glm::vec2& closest_hit_uv) const
	{
		point3 t{};
		vec3 normal{};
		glm::vec2 uv{};
		for (const auto& object : objects)
		{
			if (object->intersect(incident_ray, t, normal, uv))
			{
				if (t.z > closest_hit_t.z)
				{
					closest_hit_normal = normal;
					closest_hit_t = t;
					closest_hit_object = object;
					closest_hit_uv = uv;
				}
			}
		}
	}

	[[nodiscard]] color3 compute_color(const ray& incident_ray, const uint32_t max_rays) const
	{
		//break at max specified recursion depth to prevent stack overflow.
		if (max_rays == 0) { return {0.0f, 0.0f, 0.0f}; }

		//init intersection

		i_object* closest_hit_object = nullptr;
		point3 closest_hit_t{std::numeric_limits<float>::lowest()};
		vec3 closest_hit_normal{};
		glm::vec2 closest_hit_uv{};

		//find closest intersection
		compute_closest_intersection(incident_ray, closest_hit_object, closest_hit_t, closest_hit_normal,
		                             closest_hit_uv);

		//find total illumination and generate reflect/refract ray at intersection 
		if (closest_hit_object != nullptr)
		{
			color3 local_color;
			compute_local_illumination(closest_hit_object, closest_hit_t, closest_hit_normal, closest_hit_uv,
			                           local_color);

			return compute_global_illumination(incident_ray, max_rays, closest_hit_object, closest_hit_t,
			                                   closest_hit_normal,
			                                   closest_hit_uv, local_color) * closest_hit_object->color_at(closest_hit_t,
                                                                                                              closest_hit_uv);
		}
		return {0.005f, 0.01f, 0.0175f};
	}


private:
	std::vector<i_object*> objects{};
	std::vector<const i_light*> lights{};
};
