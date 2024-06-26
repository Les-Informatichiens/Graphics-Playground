#pragma once



class sphere final : public i_object
{
public:
	sphere(const point3& p_center, const float p_radius, const i_material* p_material) : center(p_center),
		radius(p_radius),
		radius2(glm::length2(radius)), material(p_material)
	{
	}

	bool intersect(const ray& p_ray, point3& t, vec3& normal, glm::vec2& uv) const override
	{
		const point3 acne_corrected_origin = p_ray.move(0.001f); //cetaphil
		return glm::intersectRaySphere(acne_corrected_origin, glm::normalize(p_ray.get_direction()), center, radius, t, normal);
	}

	bool alter_ray_direction(const ray& incident_ray, const vec3& normal, vec3& next_direction) const override
	{
		return material->alter_ray_direction(incident_ray, normal, next_direction);
	}

	color3 color_at(const point3& t, const glm::vec2& uv) const override
	{
		return material->color_at(t, uv);
	}

        [[nodiscard]] float get_shininess() const override
        {
            return material->get_shininess();
        }
private:
	const point3 center{};
	float radius{};
	const float radius2{};
	const i_material* material;
};
