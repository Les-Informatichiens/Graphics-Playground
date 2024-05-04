#pragma once

class i_texture
{
public:
	virtual ~i_texture() = default;
	virtual color3 color_at(const point3& p, const glm::vec2& uv) const = 0;
};
