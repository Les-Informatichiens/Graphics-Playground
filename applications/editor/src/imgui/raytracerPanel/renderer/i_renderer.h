#pragma once
class i_renderer
{
public:
	virtual ~i_renderer() = default;
	virtual bool render(uint32_t& current_compute_unit) = 0;
};
