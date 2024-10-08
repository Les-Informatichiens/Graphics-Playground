#pragma once
#include "i_object.h"
#include "glm/gtx/norm.hpp"
#include "glm/gtx/intersect.hpp"
#include "materials/i_material.h"
class i_object
{
public:
	virtual ~i_object() = default;
	virtual bool intersect(const ray& ray, point3& t, vec3& normal, glm::vec2& uv) const = 0;
	virtual bool alter_ray_direction(const ray& incident_ray, const vec3& normal, vec3& next_direction) const = 0;
	virtual color3 color_at(const point3& t, const glm::vec2& uv) const = 0;
    virtual float get_shininess() const = 0;
};
