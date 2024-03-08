//
// Created by Jonathan Richard on 2024-03-08.
//

#pragma once


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
};

}
