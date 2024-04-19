#include "rt.h"
//
// Created by Jean on 4/18/2024.
//
#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
// Optional. define TINYOBJLOADER_USE_MAPBOX_EARCUT gives robust trinagulation. Requires C++11
#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#include "object_loader/tiny_obj_loader.h"

void RayTracer::load(const char* p_file_name, std::vector<std::vector<point3>>& triangles)
{
    {

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
                triangles.emplace_back(verteses);

                index_offset += fv;
            }
        }
    }
}
RayTracer::RayTracer() : rgb_image(
        3, 480 * 1, 240 * 1, 9
)
{
}
