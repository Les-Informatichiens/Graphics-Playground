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
		glm::vec3 add = color * intensity;
		add /= (4 * glm::pi<float>() * pow(distance, 2.0f));
		return add;
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
