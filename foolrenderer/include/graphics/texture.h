#pragma once
#include <cstdint>
#include <vector>
#include "color.h"
#include "rmath/rvector.h"

enum class texture_format : uint8_t {
    ///
    /// The format has only an R component, the type is 8-bit unsigned integer.
    ///
    TEXTURE_FORMAT_R8,
    ///
    /// The components included in this format are R, G, B, and each component
    /// is an 8-bit unsigned integer type.
    ///
    TEXTURE_FORMAT_RGB8,
    ///
    /// The components included in this format are R, G, B, each component is an
    /// 8-bit unsigned integer type. And the three components are considered to
    /// be encoded in the sRGB color space.
    ///
    TEXTURE_FORMAT_SRGB8,
    ///
    /// The components included in this format are R, G, B, A, and each
    /// component is an 8-bit unsigned integer type.
    ///
    TEXTURE_FORMAT_RGBA8,
    ///
    /// The components included in this format are R, G, B, A, each component is
    /// an 8-bit unsigned integer type. And the color values of the three
    /// components of R, G, and B are considered to be encoded in the sRGB color
    /// space.
    ///
    TEXTURE_FORMAT_SRGB8_A8,
    ///
    /// The format used to store depth information, the type is float.
    ///
    TEXTURE_FORMAT_DEPTH_FLOAT
};

/*
    \brief A texture is an object that saves image pixel data in a specific format.
           The first pixel corresponds to the bottom-left corner of the texture image.
*/
struct Texture
{
    texture_format m_format;
    uint32_t m_width, m_height;

    // Consider this container as array-of-bytes
    // The actual type (float/uint8_t) depends on m_format
    std::vector<uint8_t> pixels;

    /*
        \brief Creates a texture.
        \param width The width of the texture.
        \param height The height of the texture.
    */
    Texture(texture_format form, uint32_t width, uint32_t height);

    ~Texture() = default;

    Texture(const Texture& other);

    Texture(Texture&& other) noexcept;

    Texture& operator=(Texture&& other) noexcept;

    Texture& operator=(const Texture& other);

    /*
        Move given vector data to texture pixels
    */
    void set_texture_pixels(std::vector<uint8_t>&& pixels);
    
    /*
        Copy given raw array to texture pixels
        Be careful that the size of the array should be pixel_size * width * height
    */
    bool set_texture_pixels(void* pixels);

    /* 
        Returns the raw pointer of the pixels (as bytes array)
        Do not free/delete this pointer
    */
    uint8_t* get_pixels();

    const uint8_t* get_pixels() const;

    /*
        \brief Samples pixel from the texture.
            If the texture's m_format is sRGB encoded, the function will inverse-correct
            pixel values to linear color space.

        \param texture Pointer to the texture to retrieve.
        \param texcoord Texture coordinate at which the texture will be sampled.
        \return Returns pixel on success. Returns fallback pixel on failure.
    */
    vec4 sample(vec2 texcoord) const;
};