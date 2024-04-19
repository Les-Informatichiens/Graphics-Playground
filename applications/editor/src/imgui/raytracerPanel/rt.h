//
// Created by Jean
//

#pragma once

#include <iostream>
#include <string>

#include "renderer/scene/basic_scene.h"
#include "renderer/scene/camera/i_camera.h"
#include "renderer/scene/camera/positionable_camera.h"
#include "renderer/scene/image/rgb_image.h"
#include "renderer/scene/object_manager.h"
#include "renderer/scene/objects/lights/point_light.h"
#include "renderer/scene/objects/materials/glass.h"
#include "renderer/scene/objects/materials/i_material.h"

#include "renderer/scene/objects/materials/metal.h"
#include "renderer/scene/objects/materials/textures/base_color.h"
#include "renderer/scene/objects/materials/textures/checker.h"
#include "renderer/scene/objects/sphere.h"
#include "renderer/scene/objects/triangle_mesh.h"
#include "renderer/threaded_cpu_renderer.h"

class RayTracer
{

public:
    RayTracer();
    int run()
    {

        const uint32_t compute_units = scene.horizontal_pixel_count() * scene.vertical_pixel_count() / (8*8) ;

        if (current_compute_unit < compute_units && renderer.render(current_compute_unit))
        {

            return 0;
        }


        return 1;
    }
    uint32_t current_compute_unit = 1;
    rgb_image rgb_image;
    object_manager scene_objects;
    positionable_camera camera = positionable_camera({0.0f, 2.5f, 0.0f}, {0, 1.5f, -1}, {0, 1, 0}, 90, 16.0f / 9.0f);
    basic_scene scene = basic_scene{rgb_image, camera, scene_objects};
    threaded_cpu_renderer renderer = threaded_cpu_renderer{scene};
    void load();
};
