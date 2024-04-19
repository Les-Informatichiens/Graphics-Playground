#include "rt.h"
//
// Created by Jean
//
#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
// Optional. define TINYOBJLOADER_USE_MAPBOX_EARCUT gives robust trinagulation. Requires C++11
#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#include "object_loader/tiny_obj_loader.h"

void RayTracer::load()
{
    std::vector<std::vector<point3>> mesh_data;


        const std::string p_file_name = "assets/test/teapot.obj";

        tinyobj::ObjReaderConfig reader_config;
        reader_config.mtl_search_path = "./"; // Path to material files

        tinyobj::ObjReader reader;

        if (!reader.ParseFromFile(p_file_name, reader_config))
        {
            if (!reader.Error().empty())
            {
                std::cerr << "TinyObjReader: " << reader.Error();
            }
            exit(1);
        }

        if (!reader.Warning().empty())
        {
            std::cout << "TinyObjReader: " << reader.Warning();
        }

        auto& attrib = reader.GetAttrib();
        auto& shapes = reader.GetShapes();
        //Loop over shapes
        for (const auto& shape : shapes)
        {
            // Loop over faces(polygon)
            size_t index_offset = 0;
            for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++)
            {
                const size_t fv = shape.mesh.num_face_vertices[f];

                point3 vertex{};
                std::vector<point3> verteses;
                // Loop over vertices in the face.
                for (size_t v = 0; v < fv; v++)
                {
                    // access to vertex
                    const tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
                    vertex.x = attrib.vertices[3 * static_cast<size_t>(idx.vertex_index) + 0];
                    vertex.y = attrib.vertices[3 * static_cast<size_t>(idx.vertex_index) + 1] - 1.5f;
                    vertex.z = attrib.vertices[3 * static_cast<size_t>(idx.vertex_index) + 2] - 3.0f;


                    verteses.emplace_back(vertex);
                }
                mesh_data.emplace_back(verteses);

                index_offset += fv;
            }
        }


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

    scene_objects.add_object(new sphere{{1.1f, 0.5f, -1.0f}, 0.3f, sphere_material});
    scene_objects.add_object(new triangle_mesh{mesh_material, static_cast<int>(mesh_data.size()), mesh_data});

    scene_objects.add_object(new triangle_mesh{
            floor_material, static_cast<int>(floor_triangles.size()), floor_triangles});

    scene_objects.add_light(light);
}
RayTracer::RayTracer() : rgb_image(
                                 3, 1920 * 1, 1080 * 1, 9)

{
    load();
    positionable_camera camera = positionable_camera({0.0f, 2.5f, 0.0f}, {0, 1.5f, -1}, {0, 1, 0}, 90, 16.0f / 9.0f);
    scene = basic_scene{rgb_image, camera, scene_objects};
}
