#include "rt.h"
//
// Created by Jean
//
#define TINYOBJLOADER_IMPLEMENTATION// define this in only *one* .cc
// Optional. define TINYOBJLOADER_USE_MAPBOX_EARCUT gives robust trinagulation. Requires C++11
#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#include "object_loader/tiny_obj_loader.h"

void RayTracer::load()
{


    std::vector<std::vector<Vertex>> mesh_data;

    const std::string p_file_name = "assets/test/teapot.obj";

    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = "./";// Path to material files

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
    for (const auto& shape: shapes)
    {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++)
        {
            const size_t fv = shape.mesh.num_face_vertices[f];

            point3 vertex{};
            std::vector<Vertex> vertices;
            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++)
            {
                // access to vertex
                const tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
                vertex.x = attrib.vertices[3 * static_cast<size_t>(idx.vertex_index) + 0];
                vertex.y = attrib.vertices[3 * static_cast<size_t>(idx.vertex_index) + 1] - 1.5f;
                vertex.z = attrib.vertices[3 * static_cast<size_t>(idx.vertex_index) + 2] - 3.0f;

                // access to normal
                vec3 normal;
                if (idx.normal_index >= 0)// Check if this vertex has a normal
                {
                    normal.x = attrib.normals[3 * static_cast<size_t>(idx.normal_index) + 0];
                    normal.y = attrib.normals[3 * static_cast<size_t>(idx.normal_index) + 1];
                    normal.z = attrib.normals[3 * static_cast<size_t>(idx.normal_index) + 2];
                }

                // Store the vertex and normal in your data structure
                vertices.emplace_back(Vertex{vertex, normal});
            }
            mesh_data.emplace_back(vertices);

            index_offset += fv;
        }
    }


    std::vector<std::vector<Vertex>> floor_triangles;
    std::vector<point3> tri;
    std::vector<point3> tri2;
    std::vector<point3> tri3;

    tri.emplace_back(5.5, -2, -5.5);
    tri.emplace_back(-5.5, -2, -5.5);
    tri.emplace_back(-5.5, -2, 5);
    tri2.emplace_back(5.5, -2, -5.5);
    tri2.emplace_back(-5.5, -2, 5);
    tri2.emplace_back(5.5, -2, 5);
    tri3.emplace_back(5.5, -2, -5.5);
    tri3.emplace_back(5.5, -2, 5);
    tri3.emplace_back(5.5, 11, -5.5);

    std::vector<Vertex> triangle1;
    std::vector<Vertex> triangle2;
    std::vector<Vertex> triangle3;
    for (const auto& point: tri3)
    {
        triangle3.emplace_back(Vertex{point, vec3(0, 1, 0)});
    }
    for (const auto& point: tri)
    {
        triangle1.emplace_back(Vertex{point, vec3(0, 1, 0)});
    }

    for (const auto& point: tri2)
    {
        triangle2.emplace_back(Vertex{point, vec3(0, 1, 0)});
    }

    floor_triangles.emplace_back(triangle1);
    floor_triangles.emplace_back(triangle2);
    floor_triangles.emplace_back(triangle3);

    std::vector<vec3> floor_normals;

    for (const auto& triangle: floor_triangles)
    {
        // Calculate the vectors representing two sides of the triangle
        vec3 edge1 = triangle[1].position - triangle[0].position;
        vec3 edge2 = triangle[2].position - triangle[0].position;

        // Calculate the normal by taking the cross product of the two edges
        vec3 normal = glm::normalize(glm::cross(edge1, edge2));

        // Store the normal in the normals vector
        floor_normals.push_back(normal);
    }
    for (auto& [position, normal]: triangle1)
    {
        normal = floor_normals[0];
    }

    for (auto& [position, normal]: triangle2)
    {
        normal = floor_normals[1];
    }
    for (auto& [position, normal]: triangle3)
    {
        normal = floor_normals[2];
    }

    //scene init
    const i_texture* mesh_texture = new base_color{new color3{0.5f, 0.0f, 0.0f}};
    const i_texture* floor_texture = new checker{new color3{0.0f, 0.0f, 0.0f}, new color3{1, 1, 1}};
    const i_texture* sphere_texture = new base_color{new const color3{1.0f, 1.0f, 1.0f}};
    const i_texture* sphere_texture2 = new base_color{new const color3{0.0f, 0.0f, 0.6f}};
    const i_texture* sphere_texture3 = new checker{new color3{1.0f, 1.0f, 0.0f}, new color3{0, 1, 1}};

    const i_material* mesh_material = new metal{mesh_texture, 0.0f, 10};
    const i_material* floor_material = new metal{floor_texture, 0.01f, 1.0f};
    const i_material* sphere_material = new metal{sphere_texture, 0.0f, 0.5f};
    const i_material* sphere_material2 = new metal{sphere_texture3, 0.0f, 0};
    const i_material* sphere_material3 = new glass{sphere_texture, 1.52f, 250.0f};

    const i_light* light = new point_light({-5.0f, 2.0f, 0.0f}, {1, 1, 1}, 25.0f);
    const i_light* light2 = new point_light({-1.0f, 2.0f, 0.0f}, {1, 1, 1}, 1.0f);

    scene_objects.add_object(new sphere{{1.5f, 0.3f, -1.2f}, 0.3f, sphere_material3});
    //scene_objects.add_object(new sphere{{1.5f, 1.3f, -3.2f}, 0.3f, sphere_material2});
    scene_objects.add_object(new sphere{{-1.5f, 0.7f, -2.2f}, 0.3f, sphere_material3});

    scene_objects.add_object(new triangle_mesh{mesh_material, static_cast<int>(mesh_data.size()), mesh_data});

    scene_objects.add_object(new triangle_mesh{
            floor_material, static_cast<int>(floor_triangles.size()), floor_triangles});

    scene_objects.add_light(light);
    //  scene_objects.add_light(light2);
}
RayTracer::RayTracer() : rgb_image(
                                 3, 1920 * 1, 1080 * 1, 9)

{
    load();
    auto camera = positionable_camera({0.0f, 2.5f, 0.0f}, {0, 1.5f, -1}, {0, 1, 0}, 90, 16.0f / 9.0f);
    scene = basic_scene{rgb_image, camera, scene_objects};
}
