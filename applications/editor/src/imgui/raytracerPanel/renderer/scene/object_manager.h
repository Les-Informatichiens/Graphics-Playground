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

        vec3 sample_vec = sample_dir.x * tangent_x + sample_dir.y * tangent_y + sample_dir.z * normal;

        return sample_vec;
    }

    void local_illumination(const i_object* closest_hit_object, const point3& closest_hit_t,
                            const vec3& closest_hit_normal, const glm::vec2& closest_hit_uv,
                            color3& local_color, const i_light* light)
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
            color3 diffuse = std::max(0.0f, glm::dot(closest_hit_normal, direction_to_light)) * closest_hit_object->color_at(closest_hit_t, closest_hit_uv);
            // Blinn-Phong model for the specular component
            vec3 view_dir = glm::normalize(vec3{0.0f, 2.5f, 0.0f} - closest_hit_t);
            vec3 half_vec = (direction_to_light + view_dir ) / glm::length<3>(direction_to_light + view_dir);

            color3 specular = std::pow(std::max(0.0f, glm::dot(closest_hit_normal, half_vec)), closest_hit_object->get_shininess()) * closest_hit_object->color_at(closest_hit_t, closest_hit_uv);
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
        return {0.015f, 0.03f, 0.0525f}; // Background color
    }

    color3 total_color{0.0f, 0.0f, 0.0f};

    // Local illumination (direct from lights)
    for (const auto& light: lights)
    {
        color3 local_color{0.0f, 0.0f, 0.0f};
        local_illumination(closest_hit_object, closest_hit_t, closest_hit_normal, closest_hit_uv, local_color, light);
        total_color += local_color;
    }

        // Global illumination (indirect)
        constexpr int num_samples = 16;

        // Dynamic specular weight based on material shininess
        const float specular_weight = std::min(1.0f, std::max(0.1f, 1.0f - exp(-0.1f * closest_hit_object->get_shininess())));

        color3 global_illumination{0.0f, 0.0f, 0.0f};

        for (int i = 0; i < num_samples; ++i)
        {
            vec3 sample_direction;
            float specular_prob = dist(rng);

            if (specular_prob < specular_weight)
            {
                // Specular component
                closest_hit_object->alter_ray_direction(incident_ray, closest_hit_normal, sample_direction);
                ray next_ray = {closest_hit_t + EPSILON, sample_direction};
                global_illumination += compute_color(next_ray, max_rays - 1);
            }
            else
            {
                sample_direction  = sample_hemisphere(closest_hit_normal);
                ray next_ray = {closest_hit_t + EPSILON, sample_direction};
                global_illumination += compute_color(next_ray, max_rays - 1);
            }
        }
        global_illumination /= static_cast<float>(num_samples);

        global_illumination *= closest_hit_object->color_at(closest_hit_t, closest_hit_uv);

        total_color += global_illumination;

        return total_color;
}
private:
    std::vector<i_object*> objects{};
    std::vector<const i_light*> lights{};
};
