//
// Created by Jonathan Richard on 2024-02-09.
//

#pragma once

#include <cstdint>
#include <string>

enum class UniformType : uint8_t {
    Invalid = 0,
    Float,
    Float2,
    Float3,
    Float4,
    Boolean,
    Int,
    Int2,
    Int3,
    Int4,
    Mat2x2,
    Mat3x3,
    Mat4x4
};

/// Information required to be specified when binding non-block uniforms
/// Only used when binding to opengl 2.0 shaders as uniform blocks are not supported in that
/// version. Code that can use uniform blocks should use uniform blocks.
struct UniformDesc {
    std::string name;
    int location = -1;
    UniformType type = UniformType::Invalid;
    size_t numElements = 1; // number of elements for arrays
    size_t offset = 0;
    size_t elementStride = 0;
};