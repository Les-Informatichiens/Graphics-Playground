#pragma once
#include "i_texture.h"

class base_color final :
	public i_texture
{
public:
	base_color(const color3* p_color) : color(p_color)
	{
	}

	color3 color_at(const point3& p, const glm::vec2& uv) const override
	{
		return *color;
	}

private:
	const color3* color;
};
