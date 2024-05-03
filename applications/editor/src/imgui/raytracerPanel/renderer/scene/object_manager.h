#pragma once
#include <vector>

#include "glm/gtc/constants.hpp"
#include "objects/i_object.h"
#include "objects/lights/i_light.h"

#include <random>
inline std::random_device rd;
inline std::mt19937 rng(rd());
inline std::uniform_real_distribution<float> dist(0.0f, 1.0f);
constexpr float EPSILON = 0.001f;
inline void make_orthonormal_basis(const vec3& normal, vec3& tangent_x, vec3& tangent_y)
{
    if (std::abs(normal.x) > std::abs(normal.y))
    {
        tangent_x = {-normal.z, 0.0f, normal.x};
    }
    else
    {
        tangent_x = {0.0f, normal.z, -normal.y};
    }
    tangent_x = normalize(tangent_x);
    tangent_y = cross(normal, tangent_x);
}

class object_manager
{
public:
    object_manager() = default;

    ~object_manager()
    {
        for (auto& object: objects)
        {
            delete object;
            object = nullptr;
        }
        for (auto& light: lights)
        {
            delete light;
            light = nullptr;
        }
    }

    void add_object(i_object* object)
    {
        objects.push_back(object);
    }

    void add_light(const i_light* light)
    {
        lights.push_back(light);
    }

    static vec3 sample_hemisphere(const vec3& normal)
    {
        const float u1 = dist(rng);
        const float u2 = dist(rng);

        const float r = std::sqrt(u1);
        const float phi = 2 * glm::pi<float>() * u2;

        vec3 sample_dir;
        sample_dir.x = r * std::cos(phi);
        sample_dir.y = r * std::sin(phi);
        sample_dir.z = std::sqrt(std::max(0.0f, 1 - u1));

        vec3 tangent_x, tangent_y;
        make_orthonormal_basis(normal, tangent_x, tangent_y);

        return sample_dir.x * tangent_x + sample_dir.y * tangent_y + sample_dir.z * normal;
    }

    color3 global_illumination(const ray& incident_ray, const uint32_t& max_rays,
                               const i_object* closest_hit_object,
                               const point3& closest_hit_t, const vec3& closest_hit_normal,
                               const glm::vec2& closest_hit_uv,
                               const color3& local_color, const i_light* light)
    {
        if (max_rays == 0)
        {
            return {0.0f, 0.0f, 0.0f};
        }

        vec3 next_ray_direction;
        closest_hit_object->alter_ray_direction(incident_ray, closest_hit_normal, next_ray_direction);
        color3 next_color{0.0f, 0.0f, 0.0f};
        constexpr int num_samples = 16;

        for (int i = 0; i < num_samples; ++i)
        {
            next_ray_direction = sample_hemisphere(closest_hit_normal);
            ray next_ray(closest_hit_t + EPSILON, next_ray_direction);
            next_color += compute_color(next_ray, max_rays - 1);
        }
        next_color /= static_cast<float>(num_samples);

        return next_color * closest_hit_object->color_at(closest_hit_t, closest_hit_uv);
    }

    void local_illumination(const i_object* closest_hit_object, const point3& closest_hit_t,
                            const vec3& closest_hit_normal, const glm::vec2& closest_hit_uv,
                            color3& local_color, size_t max_rays, const i_light* light)
    {
        vec3 direction_to_light{};
        light->direction_to(closest_hit_t, direction_to_light);
        ray shadow_ray{closest_hit_t + EPSILON, direction_to_light};
        bool hit_something{false};
        point3 t{0.0f, 0.0f, 0.0f};
        vec3 normal{0.0f, 0.0f, 0.0f};
        glm::vec2 uv;
        for (const auto& object: objects)
        {
            if (object->intersect(shadow_ray, t, normal, uv))
            {
                hit_something = true;
                break;
            }
        }
        if (!hit_something)
        {
            // Lambertian reflectance for the diffuse component
            color3 diffuse = dot(closest_hit_normal, direction_to_light) * closest_hit_object->color_at(closest_hit_t, closest_hit_uv);
            // Blinn-Phong model for the specular component
            vec3 view_dir = normalize(vec3{0.0f, 2.5f, 0.0f} - closest_hit_t);
            vec3 half_vec = (view_dir + direction_to_light) / length(view_dir + direction_to_light);

            color3 specular = std::pow(dot(closest_hit_normal, half_vec), closest_hit_object->get_shininess()) * closest_hit_object->color_at(closest_hit_t, closest_hit_uv);
            local_color += light->emit(closest_hit_t, closest_hit_normal) * (diffuse + specular);
        }
    }
    void closest_intersection(const ray& incident_ray,
                              i_object*& closest_hit_object, point3& closest_hit_t, vec3& closest_hit_normal,
                              glm::vec2& closest_hit_uv) const
    {
        point3 t{};
        vec3 normal{};
        glm::vec2 uv{};
        for (const auto& object: objects)
        {
            if (object->intersect(incident_ray, t, normal, uv))
            {
                if (t.z > closest_hit_t.z)
                {
                    closest_hit_normal = normal;
                    closest_hit_t = t;
                    closest_hit_object = object;
                    closest_hit_uv = uv;
                }
            }
        }
    }

    color3 compute_color(const ray& incident_ray, const uint32_t max_rays)
    {
        if (max_rays == 0)
        {
            return {0.0f, 0.0f, 0.0f};
        }

        i_object* closest_hit_object = nullptr;
        point3 closest_hit_t{std::numeric_limits<float>::lowest()};
        vec3 closest_hit_normal{};
        glm::vec2 closest_hit_uv{};

        closest_intersection(incident_ray, closest_hit_object, closest_hit_t, closest_hit_normal, closest_hit_uv);

        if (closest_hit_object == nullptr)
        {
            return {0.015f, 0.03f, 0.0525f};
        }

        color3 total_color{0.0f, 0.0f, 0.0f};

        for (const auto& light: lights)
        {

            local_illumination(closest_hit_object, closest_hit_t, closest_hit_normal, closest_hit_uv, total_color, max_rays, light);

            vec3 next_direction;
            closest_hit_object->alter_ray_direction(incident_ray, closest_hit_normal, next_direction);
            ray next_ray(closest_hit_t + EPSILON, next_direction);
            total_color += compute_color(next_ray, max_rays - 1);


            total_color += global_illumination(ray(closest_hit_t, next_direction), max_rays - 1,
                                               closest_hit_object, closest_hit_t, closest_hit_normal,
                                               closest_hit_uv, total_color, light);
        }

        return total_color * closest_hit_object->color_at(closest_hit_t, closest_hit_uv);
    }

private:
    std::vector<i_object*> objects{};
    std::vector<const i_light*> lights{};
};
