//
// Created by Jonathan Richard on 2024-02-12.
//

#include "engine/Mesh.h"
#include "glm/ext/scalar_constants.hpp"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
    : vertices(std::move(vertices)), indices(std::move(indices)), bounds(glm::vec3(0.0f), glm::vec3(0.0f))
{
    recalculateBounds();
}

// pseudo C code :
/*
 *
void Mesh_normalize( Mesh *myself )
{
    Vert     *vert = myself->vert;
    Triangle *face = myself->face;

for( int i=0; i<myself->mNumVerts; i++ ) vert[i].normal = vec3(0.0f);

for( int i=0; i<myself->mNumFaces; i++ )
{
    const int ia = face[i].v[0];
    const int ib = face[i].v[1];
    const int ic = face[i].v[2];

    const vec3 e1 = vert[ia].pos - vert[ib].pos;
    const vec3 e2 = vert[ic].pos - vert[ib].pos;
    const vec3 no = cross( e1, e2 );

    vert[ia].normal += no;
    vert[ib].normal += no;
    vert[ic].normal += no;
}

for( i=0; i<myself->mNumVerts; i++ ) verts[i].normal = normalize( verts[i].normal );
}

 */
void Mesh::normalize()
{
    for (auto &vertex : vertices)
    {
        vertex.normal = glm::vec3(0.0f);
    }

    for (int i = 0; i < indices.size(); i += 3)
    {
        const int ia = indices[i];
        const int ib = indices[i + 1];
        const int ic = indices[i + 2];

        const glm::vec3 e1 = vertices[ia].position - vertices[ib].position;
        const glm::vec3 e2 = vertices[ic].position - vertices[ib].position;
        const glm::vec3 no = glm::cross(e1, e2);

        vertices[ia].normal += no;
        vertices[ib].normal += no;
        vertices[ic].normal += no;
    }

    for (auto &vertex : vertices)
    {
        vertex.normal = glm::normalize(vertex.normal);
    }
}

std::shared_ptr<Mesh> Mesh::createQuad(float size)
{
    std::vector<Vertex> vertices = {
        {{-size, -size, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{size, -size, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{size, size, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-size, size, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    };

    std::vector<uint32_t> indices = {
        0, 1, 2, 2, 3, 0,
    };

    return std::make_shared<Mesh>(vertices, indices);
}

std::shared_ptr<Mesh> Mesh::createSphere(float radius, unsigned int longitudeSegments, unsigned int latitudeSegments){
    std::vector<Mesh::Vertex> vertices;
    std::vector<uint32_t> indices;

    for (unsigned int lat = 0; lat <= latitudeSegments; ++lat) {
        float theta = lat * glm::pi<float>() / latitudeSegments;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);

        for (unsigned int lon = 0; lon <= longitudeSegments; ++lon) {
            float phi = lon * 2 * glm::pi<float>() / longitudeSegments;
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);

            glm::vec3 normal(cosPhi * sinTheta, cosTheta, sinPhi * sinTheta);
            glm::vec2 texCoord((float)lon / longitudeSegments, (float)lat / latitudeSegments);
            glm::vec3 position = normal * radius;

            vertices.push_back(Mesh::Vertex(position, normal, texCoord));
        }
    }

    // Generating indices for each triangle
    for (unsigned int lat = 0; lat < latitudeSegments; ++lat) {
        for (unsigned int lon = 0; lon < longitudeSegments; ++lon) {
            uint32_t first = (lat * (longitudeSegments + 1)) + lon;
            uint32_t second = first + longitudeSegments + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    return std::make_shared<Mesh>(vertices, indices);
}



std::shared_ptr<Mesh> Mesh::createCube(float size)
{
    std::vector<Vertex> vertices = {
        {{-size, -size, -size}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
        {{size, -size, -size}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
        {{size, size, -size}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
        {{-size, size, -size}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},

        {{-size, -size, size}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{size, -size, size}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{size, size, size}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-size, size, size}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},

        {{-size, -size, -size}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{-size, size, -size}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{-size, size, size}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        {{-size, -size, size}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},

        {{size, -size, -size}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{size, size, -size}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{size, size, size}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        {{size, -size, size}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},

        {{-size, -size, -size}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
        {{-size, -size, size}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
        {{size, -size, size}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
        {{size, -size, -size}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},

        {{-size, size, -size}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{-size, size, size}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{size, size, size}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
        {{size, size, -size}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
    };

    std::vector<uint32_t> indices = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20,
    };

    auto mesh = std::make_shared<Mesh>(vertices, indices);

//    mesh->calculateTangents();

    return mesh;
}

void Mesh::recalculateBounds() const
{
    glm::vec3 center = glm::vec3(0.0f);
    glm::vec3 min = vertices[0].position;
    glm::vec3 max = vertices[0].position;
    for (const auto& vertex: vertices)
    {
        min = glm::min(min, vertex.position);
        max = glm::max(max, vertex.position);
    }

    center = (min + max) / 2.0f;

    bounds = Bounds(center, max - min);
}

void Mesh::clear()
{
    vertices.clear();
    indices.clear();
    bounds = Bounds(glm::vec3(0.0f), glm::vec3(0.0f));
}

//void Mesh::calculateTangents()
//{
//    // calc tangents and bitangents
//    for (int i = 0; i < indices.size(); i += 3)
//    {
//        // Shortcuts for vertices
//        glm::vec3 &v0 = vertices[indices[i]].position;
//        glm::vec3 &v1 = vertices[indices[i + 1]].position;
//        glm::vec3 &v2 = vertices[indices[i + 2]].position;
//
//        // Shortcuts for UVs
//        glm::vec2 &uv0 = vertices[indices[i]].texCoords;
//        glm::vec2 &uv1 = vertices[indices[i + 1]].texCoords;
//        glm::vec2 &uv2 = vertices[indices[i + 2]].texCoords;
//
//        // Edges of the triangle : postion delta
//        glm::vec3 deltaPos1 = v1 - v0;
//        glm::vec3 deltaPos2 = v2 - v0;
//
//        // UV delta
//        glm::vec2 deltaUV1 = uv1 - uv0;
//        glm::vec2 deltaUV2 = uv2 - uv0;
//
//        float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
//        glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
//        glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;
//
//        vertices[indices[i]].tangent += tangent;
//        vertices[indices[i + 1]].tangent += tangent;
//        vertices[indices[i + 2]].tangent += tangent;
//
//        vertices[indices[i]].bitangent += bitangent;
//        vertices[indices[i + 1]].bitangent += bitangent;
//        vertices[indices[i + 2]].bitangent += bitangent;
//    }
//}
