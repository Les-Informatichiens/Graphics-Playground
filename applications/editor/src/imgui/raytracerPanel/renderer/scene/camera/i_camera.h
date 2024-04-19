#pragma once
#include "imgui/raytracerPanel/renderer/ray.h"

class i_camera
{
public:
	virtual ~i_camera() = default;
	virtual ray cast_ray(const vec3& direction) const = 0;
	virtual vec3 get_width() const = 0;
	virtual vec3 get_height() const = 0;
	virtual float get_z_to_image() const = 0;
};
