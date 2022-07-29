#pragma once

#include "graphics/texture.h"
#include "tgafunc.h"
#include <string_view>
#include <memory>

// Convert TGA image pixels to texture's pixels or texture's pixels to TGA image
// pixels. Each component of the pixel must be an 8-bit unsigned integer type,
// and the number of components of pixels must be greater than or equal to 3.
inline static void pixel_endian_inversion(uint8_t *pixel)
{
    // Swap the values of the 1st and 3rd components of a pixel.
    std::swap(pixel[0], pixel[2]);
}

// Modify the components of the TGA image pixels to the order expected by the
// texture. Only TGA images with pixel formats TGA_PIXEL_RGB24 and
// TGA_PIXEL_ARGB32 are supported.
inline static void modify_tga_image_pixel(uint8_t *image_data, tga_info *image_info)
{
    enum tga_pixel_format pixel_format = tga_get_pixel_format(image_info);
    if (pixel_format != TGA_PIXEL_RGB24 && pixel_format != TGA_PIXEL_ARGB32)
    {
        return;
    }
    uint32_t width = tga_get_image_width(image_info);
    uint32_t height = tga_get_image_height(image_info);
    for (uint32_t y = 0; y < width; y++)
    {
        for (uint32_t x = 0; x < height; x++)
        {
            uint8_t *pixel = tga_get_pixel(image_data, image_info, x, y);
            pixel_endian_inversion(pixel);
        }
    }
}

///
/// \brief Loads image data from a TGA format file.
///
/// The corresponding format texture object will be created according to the
/// pixel format of the current image. Only supports TGA images in the format
/// TGA_PIXEL_BW8, TGA_PIXEL_RGB24 and TGA_PIXEL_ARGB32.
///
/// \param filename The TGA file to load.
/// \param is_srgb_encoding Whether the pixel value is sRGB encoded.
/// \return Returns a texture pointer on success, null pointer on failure.
///
inline std::unique_ptr<Texture> load_image(std::string_view filename, bool is_srgb_encoding)
{
    if (filename.empty())
    {
        return nullptr;
    }
    uint8_t *image_data;
    tga_info *image_info;
    enum tga_error error_code;
    error_code = tga_load(&image_data, &image_info, filename.data());
    if (error_code != TGA_NO_ERROR)
    {
        return nullptr;
    }
    enum tga_pixel_format image_pixel_format = tga_get_pixel_format(image_info);
    uint32_t width = tga_get_image_width(image_info);
    uint32_t height = tga_get_image_height(image_info);
    // The coordinate system used by the loaded image data and the coordinate
    // system used by the texture are opposite on the Y axis. So need to flip
    // the image in the Y-axis direction.
    tga_image_flip_v(image_data, image_info);

    std::unique_ptr<Texture> texture{nullptr};
    if (image_pixel_format == TGA_PIXEL_BW8)
    {
        texture = std::make_unique<Texture>(texture_format::TEXTURE_FORMAT_R8, width, height);
        if (texture != NULL)
        {
            texture->set_texture_pixels(image_data);
        }
    }
    else if (image_pixel_format == TGA_PIXEL_RGB24)
    {
        modify_tga_image_pixel(image_data, image_info);
        enum texture_format texture_format =
            is_srgb_encoding ? texture_format::TEXTURE_FORMAT_SRGB8 : texture_format::TEXTURE_FORMAT_RGB8;
        texture = std::make_unique<Texture>(texture_format, width, height);
        if (texture != NULL)
        {
            texture->set_texture_pixels(image_data);
        }
    }
    else if (image_pixel_format == TGA_PIXEL_ARGB32)
    {
        modify_tga_image_pixel(image_data, image_info);
        enum texture_format texture_format =
            is_srgb_encoding ? texture_format::TEXTURE_FORMAT_SRGB8_A8 : texture_format::TEXTURE_FORMAT_RGBA8;
        texture = std::make_unique<Texture>(texture_format, width, height);
        if (texture != NULL)
        {
            texture->set_texture_pixels(image_data);
        }
    }

    tga_free_data(image_data);
    tga_free_info(image_info);
    return texture;
}

///
/// \brief Saves the texture as a TGA format file.
///
/// Only supports textures in the format TEXTURE_FORMAT_RGB8,
/// TEXTURE_FORMAT_SRGB8, TEXTURE_FORMAT_SRGB8_A8 and TEXTURE_FORMAT_RGBA8.
///
/// When saving the alpha channel, if the texture does not contain alpha channel
/// data, the alpha channel in the saved image is all set to 0xFF.
///
/// \param texture Pointer to the texture to save.
/// \param filename The file to save to.
/// \param alpha Whether to save the alpha channel.
/// \return Returns true on success, false on failure.
///
inline bool save_image(const Texture &texture, std::string_view filename, bool alpha)
{
    size_t texture_pixel_size;
    auto texture_format = texture.m_format;
    if (texture_format == texture_format::TEXTURE_FORMAT_RGB8 ||
        texture_format == texture_format::TEXTURE_FORMAT_SRGB8)
    {
        texture_pixel_size = 3;
    }
    else if (texture_format == texture_format::TEXTURE_FORMAT_RGBA8 ||
             texture_format == texture_format::TEXTURE_FORMAT_SRGB8_A8)
    {
        texture_pixel_size = 4;
    }
    else
    {
        return false;
    }

    uint32_t texture_width = texture.m_width;
    uint32_t texture_height = texture.m_height;
    const uint8_t *texture_data = texture.get_pixels();

    size_t image_pixel_size;
    enum tga_pixel_format image_format;
    if (alpha)
    {
        image_pixel_size = 4;
        image_format = TGA_PIXEL_ARGB32;
    }
    else
    {
        image_pixel_size = 3;
        image_format = TGA_PIXEL_RGB24;
    }
    uint8_t *image_data;
    tga_info *image_info;
    tga_create(&image_data, &image_info, texture_width, texture_height,
               image_format);

    // Copy the color buffer data to the TGA image.
    for (uint32_t y = 0; y < texture_height; y++)
    {
        for (uint32_t x = 0; x < texture_width; x++)
        {
            const uint8_t *texture_pixel =
                texture_data + (y * texture_width + x) * texture_pixel_size;
            uint8_t *image_pixel = tga_get_pixel(image_data, image_info, x, y);
            for (size_t i = 0; i < image_pixel_size; i++)
            {
                if (i == texture_pixel_size)
                {
                    // That is, i == 3 && texture_pixel_size == 3.
                    // The texture has no alpha channel, the alpha channel of
                    // image pixels is set to 0xFF.
                    image_pixel[i] = 0xFF;
                }
                else
                {
                    image_pixel[i] = texture_pixel[i];
                }
            }
            // Convert the pixel components to the desired arrangement order of
            // the TGA image.
            pixel_endian_inversion(image_pixel);
        }
    }
    // For the same reason as inverting the image data vertically in the
    // load_image() function.
    tga_image_flip_v(image_data, image_info);
    tga_save_from_info(image_data, image_info, filename.data());

    tga_free_data(image_data);
    tga_free_info(image_info);
    return true;
}

// overload for Texture*
inline bool save_image(const Texture *texture, std::string_view filename, bool alpha)
{
    size_t texture_pixel_size;
    auto texture_format = texture->m_format;
    if (texture_format == texture_format::TEXTURE_FORMAT_RGB8 ||
        texture_format == texture_format::TEXTURE_FORMAT_SRGB8)
    {
        texture_pixel_size = 3;
    }
    else if (texture_format == texture_format::TEXTURE_FORMAT_RGBA8 ||
             texture_format == texture_format::TEXTURE_FORMAT_SRGB8_A8)
    {
        texture_pixel_size = 4;
    }
    else
    {
        return false;
    }

    uint32_t texture_width = texture->m_width;
    uint32_t texture_height = texture->m_height;
    const uint8_t *texture_data = texture->get_pixels();

    size_t image_pixel_size;
    enum tga_pixel_format image_format;
    if (alpha)
    {
        image_pixel_size = 4;
        image_format = TGA_PIXEL_ARGB32;
    }
    else
    {
        image_pixel_size = 3;
        image_format = TGA_PIXEL_RGB24;
    }
    uint8_t *image_data;
    tga_info *image_info;
    tga_create(&image_data, &image_info, texture_width, texture_height,
               image_format);

    // Copy the color buffer data to the TGA image.
    for (uint32_t y = 0; y < texture_height; y++)
    {
        for (uint32_t x = 0; x < texture_width; x++)
        {
            const uint8_t *texture_pixel =
                texture_data + (y * texture_width + x) * texture_pixel_size;
            uint8_t *image_pixel = tga_get_pixel(image_data, image_info, x, y);
            for (size_t i = 0; i < image_pixel_size; i++)
            {
                if (i == texture_pixel_size)
                {
                    // That is, i == 3 && texture_pixel_size == 3.
                    // The texture has no alpha channel, the alpha channel of
                    // image pixels is set to 0xFF.
                    image_pixel[i] = 0xFF;
                }
                else
                {
                    image_pixel[i] = texture_pixel[i];
                }
            }
            // Convert the pixel components to the desired arrangement order of
            // the TGA image.
            pixel_endian_inversion(image_pixel);
        }
    }
    // For the same reason as inverting the image data vertically in the
    // load_image() function.
    tga_image_flip_v(image_data, image_info);
    tga_save_from_info(image_data, image_info, filename.data());

    tga_free_data(image_data);
    tga_free_info(image_info);
    return true;
}