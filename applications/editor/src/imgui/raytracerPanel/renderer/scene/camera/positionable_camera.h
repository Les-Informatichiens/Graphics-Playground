#pragma once
#include "i_camera.h"
#include "../../ray.h"
#include "glm/glm.hpp"

class positionable_camera final : public i_camera
{
public:
	positionable_camera(const point3 look_from, const point3 look_at, const vec3 vup, const float vfov,
	                    const float p_aspect_ratio) :
		viewport_height(2 * glm::tan(glm::radians(vfov) / 2)), viewport_width(viewport_height * p_aspect_ratio),
		camera_to_world_w(normalize(look_from - look_at)),
		camera_to_world_u(normalize(cross(vup, camera_to_world_w))),
		camera_to_world_v(cross(camera_to_world_w, camera_to_world_u)),
		canvas_width(viewport_width * camera_to_world_u), canvas_height(viewport_height * camera_to_world_v),
		camera_position(look_from),
		canvas_bottom_left(camera_position - canvas_width * 0.5f - canvas_height * 0.5f - camera_to_world_w)
	{
	} //vup is viewup vector(orthonormal coordinates), vfov is vertical field of view


	~positionable_camera() override = default;


	ray cast_ray(const vec3& direction) const override
	{
		return {
			camera_position,
			canvas_bottom_left + direction.x * canvas_width + direction.y * canvas_height - camera_position
		};
	}

	vec3 get_width() const override
	{
		return canvas_width;
	}

	vec3 get_height() const override
	{
		return canvas_height;
	}

	float get_z_to_image() const override
	{
		return z_to_image;
	}

private:
	const float z_to_image = -1.0f;
	/* Distance from pinhole positionable_camera to film. Higher value makes for a narrower fov.
									   The other parameter for fov is film size (in our case the image size). Higher value makes for a wider fov.
									   This is an arbitrary value, it being 1 is only for simplification of calculations(this is the focal length). */


	const float viewport_height; //Value of 2 units to fit Normalized Device Coordinates [-1:1]
	const float viewport_width;

	vec3 camera_to_world_w;
	vec3 camera_to_world_u;
	vec3 camera_to_world_v;

	const vec3 canvas_width;
	const vec3 canvas_height;
	const point3 camera_position; //Negative z goes towards the scene.
	const point3 canvas_bottom_left;
	const point3 canvas_top_right{
		camera_position + viewport_width / 2.0f + viewport_height / 2.0f - vec3{0, 0, z_to_image}
	};
};
