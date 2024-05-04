#pragma once
#include "base_color.h"
#include "i_texture.h"
#include "../../../../utils.h"
#include "glm/glm.hpp"

class checker final :
	public i_texture
{
public:
	checker() = default;

	checker(i_texture* even, i_texture* odd)
		: even(even), odd(odd)
	{
	}

	checker(color3* even_color, color3* odd_color)
		: even(new base_color(even_color)), odd(new base_color(odd_color)), odd_color(odd_color), even_color(even_color)
	{
	}

	color3 color_at(const point3& p, const glm::vec2& uv) const override
	{
		const float sines = glm::sin(10.0f * p.x) * glm::sin(10.0f * p.y) * glm::sin(10.0f * p.z);
		if (sines < 0)
			return odd->color_at(p, uv);
		return even->color_at(p, uv);
	}

	~checker() override
	{
		delete even_color;
		delete odd_color;
		even_color = nullptr;
		odd_color = nullptr;
	}

private:
	i_texture* even{};
	i_texture* odd{};
	color3* odd_color{};
	color3* even_color{};
};
