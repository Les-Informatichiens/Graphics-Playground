#pragma once

class i_light
{
public:
	virtual ~i_light() = default;
	virtual color3 emit(const point3& to, const vec3& surface_normal) const = 0;
	virtual void direction_to(const point3& start_position, vec3& direction) const = 0;
};
