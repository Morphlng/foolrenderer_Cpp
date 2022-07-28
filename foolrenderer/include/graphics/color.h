#pragma once

#include <cstdint>
#include <cmath>

constexpr auto GAMMA = (2.2f);

// The input value should be ranged from [0,1]
inline uint8_t float_to_uint8(float value) { return value * 0xFF; }

inline float uint8_to_float(uint8_t value) { return value / 255.0f; }

///
/// \brief Converts a linear color component to sRGB space.
///
/// \param value The R, G or B component of a linear color.
/// \return Returns the converted value.
///
inline float convert_to_srgb_color(float value) {
    // This is just an approximate conversion method. A discussion of nonlinear
    // color space can be found in NVIDIA's GPU Gems 3:
    // https://developer.nvidia.com/gpugems/gpugems3/part-iv-image-effects/chapter-24-importance-being-linear
    return powf(value, 1.0f / GAMMA);
}

///
/// \brief Converts a sRGB color component to linear space.
///
/// \param vlaue The R, G or B component of a sRGB color.
/// \return Returns the converted value.
///
inline float convert_to_linear_color(float vlaue) {
    // Similar to the convert_to_srgb_color() function, this is just an
    // approximate conversion.
    return powf(vlaue, GAMMA);
}