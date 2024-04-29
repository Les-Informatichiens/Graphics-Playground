#pragma once
#include <vector>

#include "../utils.h"
#include "glm/gtc/constants.hpp"
#include "objects/i_object.h"
#include "objects/lights/i_light.h"

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

    color3 global_illumination(const ray& incident_ray, const uint32_t& max_rays,
                           const i_object* closest_hit_object,
                           const point3& closest_hit_t, const vec3& closest_hit_normal,
                           const glm::vec2& closest_hit_uv,
                           const color3& local_color)
    {

        vec3 next_ray_direction;
        // Check if the material wants to alter the ray direction
        if (closest_hit_object->alter_ray_direction(incident_ray, closest_hit_normal, next_ray_direction))
        {
            // Calculate the next color in the path
            ray next_ray(closest_hit_t, next_ray_direction);
            color3 next_color = compute_color(next_ray, max_rays - 1);

            // Simulate a simple reflection model using shininess as a glossy factor
            float shininess = closest_hit_object->get_shininess();
            float specular_coefficient = std::pow(glm::dot(next_ray_direction, -incident_ray.get_direction()), shininess);

            // Combine local color with reflected color
            return local_color + specular_coefficient * next_color;
        }
        return local_color;
    }

    void local_illumination(const i_object* closest_hit_object, const point3& closest_hit_t,
                                    const vec3& closest_hit_normal, const glm::vec2& closest_hit_uv,
                                    color3& local_color) const
    {
        for (const auto& light: lights)
        {
            vec3 direction_to_light{};
            light->direction_to(closest_hit_t, direction_to_light);
            ray shadow_ray{closest_hit_t, direction_to_light};
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
                float diffuse = dot(closest_hit_normal, direction_to_light);
                // Phong model for the specular component
                vec3 view_dir = normalize( vec3{0.0f, 2.5f, 0.0f} - closest_hit_t);
                vec3 half_vec = (view_dir+ direction_to_light) / length(view_dir + direction_to_light);

                float specular = std::pow( dot(closest_hit_normal, half_vec), closest_hit_object->get_shininess());
                local_color +=  light->emit(closest_hit_t, closest_hit_normal) * (specular + diffuse);
            }
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

    [[nodiscard]] color3 compute_color(const ray& incident_ray, const uint32_t max_rays)
    {
        //break at max specified recursion depth to prevent stack overflow.
        if (max_rays == 0) { return {0.0f, 0.0f, 0.0f}; }

        //init intersection
        i_object* closest_hit_object = nullptr;
        point3 closest_hit_t{std::numeric_limits<float>::lowest()};
        vec3 closest_hit_normal{};
        glm::vec2 closest_hit_uv{};

        //find closest intersection
        closest_intersection(incident_ray, closest_hit_object, closest_hit_t, closest_hit_normal,
                                     closest_hit_uv);

        //find total illumination and generate reflect/refract ray at intersection
        if (closest_hit_object != nullptr)
        {
            color3 local_color{0.0f, 0.0f, 0.0f};
            local_illumination(closest_hit_object, closest_hit_t, closest_hit_normal, closest_hit_uv, local_color);
            return global_illumination(incident_ray, max_rays, closest_hit_object, closest_hit_t,
                                              closest_hit_normal,
                                              closest_hit_uv, local_color) *
                   closest_hit_object->color_at(closest_hit_t,
                                                closest_hit_uv);
        }
        return {0.015f, 0.03f, 0.0525f};
    }


private:
    std::vector<i_object*> objects{};
    std::vector<const i_light*> lights{};
};
