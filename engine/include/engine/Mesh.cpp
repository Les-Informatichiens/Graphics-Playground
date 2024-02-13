//
// Created by Jonathan Richard on 2024-02-12.
//

#include "Mesh.h"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
    : vertices(std::move(vertices)), indices(std::move(indices))
{
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
