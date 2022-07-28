#include "texture.h"

inline static size_t get_pixel_size(texture_format format)
{
    switch (format) {
    case texture_format::TEXTURE_FORMAT_R8:
        return 1;
    case texture_format::TEXTURE_FORMAT_RGB8:
    case texture_format::TEXTURE_FORMAT_SRGB8:
        return 3;
    case texture_format::TEXTURE_FORMAT_RGBA8:
    case texture_format::TEXTURE_FORMAT_SRGB8_A8:
        return 4;
    case texture_format::TEXTURE_FORMAT_DEPTH_FLOAT:
        return sizeof(float);
    default:
        return 0;
    }
}

inline static bool is_srgb_encoding(texture_format format) {
    if (format == texture_format::TEXTURE_FORMAT_SRGB8 || format == texture_format::TEXTURE_FORMAT_SRGB8_A8) {
        return true;
    }
    return false;
}

Texture::Texture(texture_format form, uint32_t width, uint32_t height)
    : m_format(form), m_width(width), m_height(height) {
    if (m_width == 0 || m_height == 0) {
        return;
    }

    size_t pixel_size = get_pixel_size(m_format);
    if (pixel_size == 0) {
        return;
    }

    pixels.resize(pixel_size * m_width * m_height);
}

Texture::Texture(const Texture& other)
    : m_format(other.m_format), m_width(other.m_width), m_height(other.m_height), pixels(other.pixels) {}

Texture::Texture(Texture&& other) noexcept
    : m_format(other.m_format), m_width(other.m_width), m_height(other.m_height), pixels(std::move(other.pixels)) {}

Texture& Texture::operator=(Texture&& other) noexcept
{
    this->m_format = other.m_format;
    this->m_width = other.m_width;
    this->m_height = other.m_height;
    this->pixels = std::move(other.pixels);

    return *this;
}

Texture& Texture::operator=(const Texture& other)
{
    this->m_format = other.m_format;
    this->m_width = other.m_width;
    this->m_height = other.m_height;
    this->pixels = other.pixels;

    return *this;
}

/*
    Move given vector data to texture pixels
*/
void Texture::set_texture_pixels(std::vector<uint8_t>&& pixels)
{
    this->pixels = std::move(pixels);
}

/*
    Copy given raw array to texture pixels
    Be careful that the size of the array should be pixel_size * width * height
*/
bool Texture::set_texture_pixels(void* pixels)
{
    size_t pixel_size = get_pixel_size(m_format);
    if (pixel_size == 0) {
        return false;
    }

    std::memcpy(this->pixels.data(), pixels, pixel_size * m_width * m_height);
    return true;
}

/*
    Returns the raw pointer of the pixels (as bytes array)
    Do not free/delete this pointer
*/
uint8_t* Texture::get_pixels() {
    return pixels.data();
}

const uint8_t* Texture::get_pixels() const {
    return pixels.data();
}

/*
    \brief Samples pixel from the texture.
        If the texture's m_format is sRGB encoded, the function will inverse-correct
        pixel values to linear color space.

    \param texture Pointer to the texture to retrieve.
    \param texcoord Texture coordinate at which the texture will be sampled.
    \return Returns pixel on success. Returns fallback pixel on failure.
*/
vec4 Texture::sample(vec2 texcoord) const {
    float u = clamp01(texcoord.u);
    float v = clamp01(texcoord.v);
    uint32_t u_index = (uint32_t)(u * m_width);
    uint32_t v_index = (uint32_t)(v * m_height);

    // Prevent array access out of bounds.
    u_index = u_index >= m_width ? m_width - 1 : u_index;
    v_index = v_index >= m_height ? m_height - 1 : v_index;
    size_t pixel_offset = (size_t)u_index + v_index * m_width;

    auto pixel = VEC4_ONE;
    auto raw_pixels = get_pixels();
    if (m_format == texture_format::TEXTURE_FORMAT_DEPTH_FLOAT) {
        const float* target = (float*)raw_pixels + pixel_offset;
        pixel.r = *target;
        pixel.g = *target;
        pixel.b = *target;
    }
    else if (m_format == texture_format::TEXTURE_FORMAT_R8) {
        const uint8_t* target = (uint8_t*)raw_pixels + pixel_offset;
        pixel.r = uint8_to_float(target[0]);
        pixel.g = pixel.r;
        pixel.b = pixel.r;
    }
    else {
        size_t pixel_size = get_pixel_size(m_format);
        if (pixel_size == 0) {
            pixel = VEC4_ZERO;
        }
        else {
            // m_format == TEXTURE_FORMAT_RGB8 ||
            // m_format == TEXTURE_FORMAT_SRGB8 ||
            // m_format == TEXTURE_FORMAT_RGBA8 ||
            // m_format == TEXTURE_FORMAT_SRGB8_A8)
            const uint8_t* target =
                (uint8_t*)raw_pixels + pixel_offset * pixel_size;
            for (size_t i = 0; i < pixel_size; i++) {
                pixel.elements[i] = uint8_to_float(target[i]);
            }
            if (is_srgb_encoding(m_format)) {
                pixel.r = convert_to_linear_color(pixel.r);
                pixel.g = convert_to_linear_color(pixel.g);
                pixel.b = convert_to_linear_color(pixel.b);
            }
        }
    }
    return pixel;
}