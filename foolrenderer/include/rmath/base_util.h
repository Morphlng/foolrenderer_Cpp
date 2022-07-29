#pragma once
#include <algorithm>

constexpr auto SMALL_ABSOLUTE_FLOAT = (1.e-8f);
constexpr auto PI = (3.1415926535897932f);

template <typename T>
T clamp(T n, T min, T max)
{
    return std::min(std::max(n, min), max);
}

template <typename T>
T clamp01(T n)
{
    // std::min/max requires same type for each element
    return std::min(std::max(n, (T)0), (T)1);
}

template <typename T>
T lerp(T a, T b, T t)
{
    return a + t * (b - a);
}