#pragma once
#include "glm/vec3.hpp"
using point3 = glm::vec3;
using color3 = glm::vec3;
using vec3 = glm::vec3;
struct Vertex
{
    point3 position;
    vec3 normal;
};