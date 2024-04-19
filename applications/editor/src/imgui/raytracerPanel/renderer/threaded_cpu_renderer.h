#pragma once

#include <future>
#include <thread>
#include "i_renderer.h"
#include "scene/i_scene.h"
#include "utils.h"

class threaded_cpu_renderer final :
	public i_renderer
{
public:
    explicit threaded_cpu_renderer(const i_scene&
                                  scene

                          ) : scene(scene)

    {
    }

    bool render(uint32_t& current_compute_unit) override //return true if an image was rendered successfully or false otherwise.
	{
		constexpr uint32_t work_unit_pixels{ 8 };


		auto get_compute_unit = [&](const uint32_t& compute_unit_top_left_corner_x_coordinates,
			const uint32_t& compute_unit_top_left_corner_y_coordinates)
			{
				{
					vec3 direction{};
					color3 pixel_color{};
					uint32_t pixel_index{};
					for (uint32_t y = compute_unit_top_left_corner_y_coordinates; y >
						compute_unit_top_left_corner_y_coordinates - work_unit_pixels;
						y--)
					{
						for (uint32_t x = compute_unit_top_left_corner_x_coordinates; x <
							compute_unit_top_left_corner_x_coordinates + work_unit_pixels; x++)
						{
							direction.x = (x + 0.5f) / scene.horizontal_pixel_count(); //camera to world
							direction.y = (y + 0.5f) / scene.vertical_pixel_count(); //

							ray ray{ scene.trace_camera_ray(direction) }; //ray from camera to world

							pixel_color = scene.color_at(ray); //ray color at intersection plus secondary rays
							pixel_index = (scene.vertical_pixel_count() - y) * scene.horizontal_pixel_count() + x;

							scene.add_color_to_image(pixel_color, pixel_index);
						}
					}
				}
			};

		const uint32_t compute_units_per_horizontal_line = scene.horizontal_pixel_count() / work_unit_pixels;
		uint32_t top_left_y{ (scene.vertical_pixel_count() - (current_compute_unit / compute_units_per_horizontal_line) * work_unit_pixels)};
		uint32_t top_left_x{(current_compute_unit - 1) % compute_units_per_horizontal_line * work_unit_pixels};
        const uint32_t end_compute_unit = current_compute_unit + work_unit_pixels;
		for (; current_compute_unit < end_compute_unit; ++current_compute_unit)
		{
			threads.emplace_back(get_compute_unit, top_left_x, top_left_y);

			if (current_compute_unit % compute_units_per_horizontal_line == 0)
			{
				top_left_x = 0;
				top_left_y -= work_unit_pixels;
				continue;
			}
			top_left_x += work_unit_pixels;
		}

		return 1;
	}

private:
    std::vector<std::jthread> threads{};
	const i_scene& scene;
};
