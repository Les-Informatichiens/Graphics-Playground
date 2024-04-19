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
#include "renderer/scene/objects/materials/textures/i_texture.h"
#include "renderer/scene/objects/sphere.h"
#include "renderer/scene/objects/triangle_mesh.h"
#include "renderer/threaded_cpu_renderer.h"

class RayTracer
{

public:
    RayTracer();
    int run()
    {

        time_t start, end;
        time(&start);
        std::ios_base::sync_with_stdio(false);

        //mesh load and init
        const auto inputfile = "assets/test/teapot.obj";
        std::vector<std::vector<point3>> mesh_data;
        load(inputfile, mesh_data);

        std::cout << std::to_string(mesh_data.size()) + " triangles" << std::endl;
        std::vector<std::vector<point3>> floor_triangles;
        std::vector<point3> tri;
        std::vector<point3> tri2;

        tri.emplace_back(-5.5, -2, -5.5);
        tri.emplace_back(5.5, -2, -5.5);
        tri.emplace_back(-5.5, -2, 5);
        tri2.emplace_back(5.5, -2, -5.5);
        tri2.emplace_back(-5.5, -2, 5);
        tri2.emplace_back(5.5, -2, 5);
        floor_triangles.emplace_back(tri);
        floor_triangles.emplace_back(tri2);

        //scene init
        const i_texture* mesh_texture = new base_color{new color3{0.5f, 0.0f, 0.0f}};
        const i_texture* floor_texture = new checker{new color3{0.0f, 0.0f, 0.0f}, new color3{1, 1, 1}};
        const i_texture* sphere_texture = new base_color{new const color3{1.0f, 1.0f, 1.0f}};

        const i_material* mesh_material = new metal{mesh_texture, 0.0f};
        const i_material* floor_material = new metal{floor_texture, 1.0f};
        const i_material* sphere_material = new glass{sphere_texture, 1.52f};

        const i_light* light = new point_light({-1.0f, 2.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, 25.0f);

        object_manager scene_objects{};
        scene_objects.add_object(new sphere{{1.1f, 0.5f, -1.0f}, 0.3f, sphere_material});
        scene_objects.add_object(new triangle_mesh{mesh_material, static_cast<int>(mesh_data.size()), mesh_data});

        scene_objects.add_object(new triangle_mesh{
                floor_material, static_cast<int>(floor_triangles.size()), floor_triangles});

        scene_objects.add_light(light);

        //initializing basic structures
        i_image& image = rgb_image;
        const i_camera& camera = positionable_camera({0.0f, 2.5f, 0.0f}, {0, 1.5f, -1}, {0, 1, 0}, 90, 16.0f / 9.0f);
        const i_scene& scene = basic_scene{image, camera, scene_objects};

        //render
        const i_renderer& renderer = threaded_cpu_renderer{scene};
        if (renderer.render())
        {
            time(&end);
            std::cout << "rendered the image in " + std::to_string(end - start) + " seconds" << std::endl;



            return 1;
        }
        std::cout << "an error occurred while rendering the image!" << std::endl;


        return 0;
    }
    rgb_image rgb_image;
    void load(const char* p_file_name, std::vector<std::vector<point3>>& triangles);
};
