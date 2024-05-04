#pragma once
#include "../../utils.h"

class i_image
{
public:
	virtual ~i_image() = default;
	virtual void append_pixel_color(const color3& color) = 0;
	virtual uint32_t get_width() const = 0;
	virtual uint32_t get_height() const = 0;
	virtual uint32_t get_max_rays_per_pixel() const = 0;
	virtual uint8_t* get_pixel_data() const = 0;
	virtual uint32_t get_nb_channels() const = 0;
	virtual void add_pixel_color_at_index(const color3& color, const uint32_t& pixel_index) const = 0;
};
