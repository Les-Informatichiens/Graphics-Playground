#pragma once
#include "../../../utils.h"

class i_material
{
public:
	virtual ~i_material() = default;
	virtual bool alter_ray_direction(const ray& incident_ray, const vec3& normal, vec3& next_direction) const = 0;
	virtual color3 color_at(const point3& t, const glm::vec2& uv) const = 0;
};
