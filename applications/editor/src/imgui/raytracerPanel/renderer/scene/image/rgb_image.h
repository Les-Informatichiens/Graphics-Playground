#pragma once
#include <cstdint>

#include "i_image.h"
#include "../../utils.h"

inline float clamp(float x, float min, float max)
{
	if (x < min) return min;
	if (x > max) return max;
	return x;
}

class rgb_image final : public i_image
{
public:
	rgb_image(const int p_nb_channels, const int p_width, const int p_height, const int p_max_rays_per_pixel) :
		nb_channels(p_nb_channels),
		width(p_width),
		height(p_height),
		aspect_ratio(
			width / static_cast<float>(height)),
		pixel_count(height * p_width * p_nb_channels),
		pixels(new uint8_t[pixel_count]), max_rays_per_pixel(p_max_rays_per_pixel)
	{
	}

	~rgb_image() override
	{
		delete[] pixels;
		pixels = nullptr;
	}

	void append_pixel_color(const color3& color) override
	{
		pixels[current_pixel_index++] = clamp(pow(color.r, 1.0f / 2.2f), 0.0f, 1.0f) * 255.0f; //float between [0-1] to RGB
		pixels[current_pixel_index++] = clamp(pow(color.g, 1.0f / 2.2f), 0.0f, 1.0f) * 255.0f;
		pixels[current_pixel_index++] = clamp(pow(color.b, 1.0f / 2.2f), 0.0f, 1.0f) * 255.0f;
	}

	void add_pixel_color_at_index(const color3& color, const uint32_t& pixel_index) const override
	{
		uint32_t rgb_pixel_index = pixel_index * 3;
		pixels[rgb_pixel_index++] = clamp(pow(color.r, 1.0f / 2.2f), 0.0f, 1.0f) * 255.0f; //float between [0-1] to RGB
		pixels[rgb_pixel_index++] = clamp(pow(color.g, 1.0f / 2.2f), 0.0f, 1.0f) * 255.0f;
		pixels[rgb_pixel_index] = clamp(pow(color.b, 1.0f / 2.2f), 0.0f, 1.0f) * 255.0f;
	}

	uint32_t get_width() const override { return width; }
	uint32_t get_height() const override { return height; }
	uint32_t get_max_rays_per_pixel() const override { return max_rays_per_pixel; }
	uint8_t* get_pixel_data() const override { return pixels; }
	uint32_t get_nb_channels() const override { return nb_channels; }

private:
	uint32_t current_pixel_index{0};
	const int nb_channels;
	const uint32_t width;
	const uint32_t height;
	const float aspect_ratio;
	const uint32_t pixel_count;
	uint8_t* pixels;
	const int max_rays_per_pixel;
};
