#pragma once
#include "../ray.h"
#include "object_manager.h"

class i_scene
{
public:
	virtual ~i_scene() = default;
	virtual ray trace_camera_ray(const vec3& direction) const = 0;
	virtual uint32_t vertical_pixel_count() const = 0;
	virtual uint32_t horizontal_pixel_count() const = 0;
	virtual vec3 viewport_height() const = 0;
	virtual vec3 viewport_width() const = 0;
	virtual float z_to_image() const = 0;
	virtual color3 color_at(const ray& ray) const = 0;
	virtual void append_color_to_image(const color3& color) const = 0;
	virtual void add_color_to_image(const color3& color, const uint32_t& pixel_index) const = 0;
	virtual uint8_t* get_image_data() const = 0;
};
