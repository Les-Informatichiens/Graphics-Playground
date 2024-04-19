#pragma once

#include "camera/i_camera.h"
#include "i_scene.h"
#include "image/i_image.h"
#include "object_manager.h"


class basic_scene : public i_scene
{
public:
	basic_scene(i_image& p_image, i_camera& p_camera, object_manager& p_scene_objects) : image(p_image),
                                                                                               scene_objects(p_scene_objects),
                                                                                               camera(p_camera)
    {
    }
    basic_scene(const basic_scene& other)
        : i_scene(other),
          image(other.image),
          scene_objects(other.scene_objects),
          camera(other.camera) {}
    basic_scene(basic_scene&& other) noexcept
        : i_scene(std::move(other)),
          image(other.image),
          scene_objects(other.scene_objects),
          camera(other.camera) {}
    basic_scene& operator=(const basic_scene& other)
    {
        if (this == &other)
            return *this;
        i_scene::operator=(other);
        image = other.image;
        scene_objects = other.scene_objects;
        camera = other.camera;
        return *this;
    }
    basic_scene& operator=(basic_scene&& other) noexcept
    {
        if (this == &other)
            return *this;
        i_scene::operator=(std::move(other));
        image = other.image;
        scene_objects = other.scene_objects;
        camera = other.camera;
        return *this;
    }
    ~basic_scene() override = default;

	ray trace_camera_ray(const vec3& direction) const override
	{
		return camera.cast_ray(direction);
	}

	uint32_t vertical_pixel_count() const override
	{
		return image.get_height();
	}

	uint32_t horizontal_pixel_count() const override
	{
		return image.get_width();
	}

	vec3 viewport_height() const override
	{
		return camera.get_height();
	}

	vec3 viewport_width() const override
	{
		return camera.get_width();
	}

	float z_to_image() const override
	{
		return camera.get_z_to_image();
	}

	color3 color_at(const ray& ray) const override
	{
		return scene_objects.compute_color(ray, image.get_max_rays_per_pixel());
	}

	void append_color_to_image(const color3& color) const override { image.append_pixel_color(color); }

	void add_color_to_image(const color3& color, const uint32_t& pixel_index) const override
	{
		image.add_pixel_color_at_index(color, pixel_index);
	}
    uint8_t* get_image_data() const override
    {
        return image.get_pixel_data();
    }

private:
	i_image& image;
	object_manager& scene_objects;
	i_camera& camera;
};
