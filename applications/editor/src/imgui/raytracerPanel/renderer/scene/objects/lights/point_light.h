#pragma once
#include "i_light.h"

class point_light final :
	public i_light
{
public:
	point_light(const point3& p_position, const color3& p_color, const float p_intensity) : position(p_position),
		color(p_color), intensity(p_intensity)
	{
	}

	[[nodiscard]] color3 emit(const point3& to, const vec3& surface_normal) const override
	{
		const float distance = glm::distance(position, to);

		return color * (1.0f / (1.0f + 0.5f * distance + 0.01f * distance * distance)) * intensity;
	}

	void direction_to(const point3& start_position, vec3& direction) const override
	{
		direction = normalize(position - start_position);
	}

private:
	const point3 position;
	const color3 color;
	const float intensity;
};
