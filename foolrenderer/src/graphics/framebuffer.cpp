#include "graphics/framebuffer.h"

inline static uint8_t clear_color[4]{0};

FrameBuffer::FrameBuffer() : m_width(0), m_height(0), color_buffer(nullptr), depth_buffer(nullptr) {}

// This macro is used in attach_texture to update width and height
#define SET_MIN_SIZE(buffer)                              \
    do                                                    \
    {                                                     \
        if (buffer)                                       \
        {                                                 \
            uint32_t buffer_width = buffer->m_width;      \
            uint32_t buffer_height = buffer->m_height;    \
            m_width = std::min(m_width, buffer_width);    \
            m_height = std::min(m_height, buffer_height); \
        }                                                 \
    } while (0)

bool FrameBuffer::attach_texture(attachment_type attachment, Texture *texture)
{
    bool result = false;
    if (texture)
    {
        switch (attachment)
        {
        case attachment_type::COLOR_ATTACHMENT:
            if (texture->m_format == texture_format::TEXTURE_FORMAT_RGBA8 ||
                texture->m_format == texture_format::TEXTURE_FORMAT_SRGB8_A8)
            {
                color_buffer.reset(texture);
                result = true;
            }
            break;
        case attachment_type::DEPTH_ATTACHMENT:
            if (texture->m_format == texture_format::TEXTURE_FORMAT_DEPTH_FLOAT)
            {
                depth_buffer.reset(texture);
                result = true;
            }
            break;
        }
    }
    else
    {
        switch (attachment)
        {
        case attachment_type::COLOR_ATTACHMENT:
            color_buffer.reset();
            result = true;
            break;
        case attachment_type::DEPTH_ATTACHMENT:
            depth_buffer.reset();
            result = true;
            break;
        }
    }

    // Update the framebuffer size.
    if (result)
    {
        if (!color_buffer && !depth_buffer)
        {
            m_width = 0;
            m_height = 0;
        }
        else
        {
            m_width = UINT32_MAX;
            m_height = UINT32_MAX;
            SET_MIN_SIZE(color_buffer);
            SET_MIN_SIZE(depth_buffer);
        }
    }
    return result;
}

bool FrameBuffer::attach_texture(attachment_type attachment, std::unique_ptr<Texture> &&texture)
{
    bool result = false;
    if (texture)
    {
        switch (attachment)
        {
        case attachment_type::COLOR_ATTACHMENT:
            if (texture->m_format == texture_format::TEXTURE_FORMAT_RGBA8 ||
                texture->m_format == texture_format::TEXTURE_FORMAT_SRGB8_A8)
            {
                color_buffer.swap(texture);
                result = true;
            }
            break;
        case attachment_type::DEPTH_ATTACHMENT:
            if (texture->m_format == texture_format::TEXTURE_FORMAT_DEPTH_FLOAT)
            {
                depth_buffer.swap(texture);
                result = true;
            }
            break;
        }
    }
    else
    {
        switch (attachment)
        {
        case attachment_type::COLOR_ATTACHMENT:
            color_buffer.reset();
            result = true;
            break;
        case attachment_type::DEPTH_ATTACHMENT:
            depth_buffer.reset();
            result = true;
            break;
        }
    }

    // Update the framebuffer size.
    if (result)
    {
        if (!color_buffer && !depth_buffer)
        {
            m_width = 0;
            m_height = 0;
        }
        else
        {
            m_width = UINT32_MAX;
            m_height = UINT32_MAX;
            SET_MIN_SIZE(color_buffer);
            SET_MIN_SIZE(depth_buffer);
        }
    }
    return result;
}

/*
    \brief Sets clear values for the color buffers.
        Sets the red, green, bule and alpha values used by clear_framebuffer() to
        clear the color buffers. The set values are clamped to the range [0,1] and
        the initial values are all 0.
    \param red The R component of the color value.
    \param green The G component of the color value.
    \param blue The B component of the color value.
    \param alpha The A component of the color value.
*/
void FrameBuffer::set_clear_color(float red, float green, float blue, float alpha)
{
    clear_color[0] = float_to_uint8(clamp01(red));
    clear_color[1] = float_to_uint8(clamp01(green));
    clear_color[2] = float_to_uint8(clamp01(blue));
    clear_color[3] = float_to_uint8(clamp01(alpha));
}

/*
    \brief Uses preset values to clear all buffers in the framebuffer.
        Each pixel of the color buffer will be cleared using the value previously
        set via the set_clear_color() function. For depth buffers, a fixed value of
        1 will be used to clear each pixel. If framebuffer is a null pointer, the
        function does nothing.

    \param framebuffer Pointer to the framebuffer to clear.
*/
void FrameBuffer::clear()
{
    size_t pixel_cnt = m_width * m_height;
    if (color_buffer)
    {
        uint8_t *pixels = color_buffer->get_pixels();
        for (size_t i = 0; i < pixel_cnt; i++)
        {
            uint8_t *pixel = pixels + i * 4;
            pixel[0] = clear_color[0];
            pixel[1] = clear_color[1];
            pixel[2] = clear_color[2];
            pixel[3] = clear_color[3];
        }
    }
    if (depth_buffer)
    {
        float *pixels = (float *)depth_buffer->get_pixels();
        for (size_t i = 0; i < pixel_cnt; i++)
        {
            pixels[i] = 1.0f;
        }
    }
}

#undef SET_MIN_SIZE