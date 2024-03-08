//
// Created by Jonathan Richard on 2024-03-08.
//

#pragma once

#include <functional>

namespace util
{

class Math
{
public:
    template<typename T, typename TT, typename TTT>
    static constexpr T clamp(T value, TT min, TTT max)
    {
        return value < min ? min : (value > max ? max : value);
    }

    template <class T>
    static inline void hash_combine(std::size_t& seed, const T& v)
    {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
    }
};

}
