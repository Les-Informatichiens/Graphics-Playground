#pragma once
#include "glm/gtx/normal.hpp"
#include "utils.h"

class ray
{
public:
	ray(const point3& p_origin, const vec3& p_direction) : origin(p_origin), direction(normalize(p_direction))
	{
	}

	point3 get_origin() const { return origin; }
	vec3 get_direction() const { return direction; }

	point3 move(const float t) const
	{
		return origin + t * direction; //P(t) = O + tD		where P is position O is origin and D is direction.
	}

private:
	const point3 origin{};
	const vec3 direction;
};
