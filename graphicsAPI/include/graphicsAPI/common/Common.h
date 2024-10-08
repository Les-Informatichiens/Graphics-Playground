//
// Created by Jonathan Richard on 2024-02-04.
//

#pragma once

#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <limits>
#include <stdexcept>
#include <iostream>
#include <assert.h>

enum class DataType
{
    None,
    Bool,
    Int,
    UInt,
    Float,
    Double,
    BVec2,
    BVec3,
    BVec4,
    IVec2,
    IVec3,
    IVec4,
    UVec2,
    UVec3,
    UVec4,
    Vec2,
    Vec3,
    Vec4,
    DVec2,
    DVec3,
    DVec4,
    Mat2,
    Mat2x3,
    Mat2x4,
    Mat3x2,
    Mat3,
    Mat3x4,
    Mat4x2,
    Mat4x3,
    Mat4,
    DMat2,
    DMat2x3,
    DMat2x4,
    DMat3x2,
    DMat3,
    DMat3x4,
    DMat4x2,
    DMat4x3,
    DMat4
};


enum class ResourceStorage
{
    Invalid,  /// Invalid sharing mode
    Private,  /// Memory private to GPU access (fastest)
    Shared,   /// Memory shared between CPU and GPU
    Managed,  /// Memory pair synchronized between CPU and GPU
    Memoryless/// Memory can be accessed only by the GPU and only exist temporarily during a render
};


enum class PrimitiveType : uint8_t
{
    Point,
    Line,
    LineStrip,
    Triangle,
    TriangleStrip,
};

enum class IndexFormat : uint8_t
{
    UInt16,
    UInt32,
};


struct Color
{
    float r;
    float g;
    float b;
    float a;

    Color(float r, float g, float b) : r(r), g(g), b(b), a(1.0f) {}
    Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}

    [[nodiscard]] const float* toFloatPtr() const
    {
        return &r;
    }
};

struct Viewport
{
    float x;
    float y;
    float width;
    float height;
    float minDepth;
    float maxDepth;
};

/// Use value-initialization (i.e. braces) to 0-initialize: `Rect<float> myRect{};`
template<typename T>
struct Rect {
private:
    static constexpr T kNullValue = std::numeric_limits<T>::has_infinity
                                            ? std::numeric_limits<T>::infinity()
                                            : std::numeric_limits<T>::max();

public:
    T x = kNullValue;
    T y = kNullValue;
    T width{}; // zero-initialize
    T height{}; // zero-initialize

    bool isNull() const {
        return kNullValue == x && kNullValue == y;
    }
};

using ScissorRect = Rect<uint32_t>;

enum ImageAccessFlags : uint8_t
{
    ReadOnly = 1 << 0,
    WriteOnly = 1 << 1,
    ReadWrite = ReadOnly | WriteOnly
};

constexpr size_t MAX_TEXTURE_SAMPLERS = 16;
constexpr size_t MAX_TEXTURE_UNITS = 16;
constexpr size_t MAX_VERTEX_BUFFERS = 32;

// Get value of enum by stripping enum class type
template<typename E>
constexpr typename std::underlying_type<E>::type EnumToValue(E enumerator) noexcept {
    return static_cast<typename std::underlying_type<E>::type>(enumerator);
}

template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}
