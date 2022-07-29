#pragma once

#include <cstdint>
#include <memory>
#include "texture.h"

enum class attachment_type : uint8_t
{
    COLOR_ATTACHMENT,
    DEPTH_ATTACHMENT
};

/*
    \brief A framebuffer is a collection of buffers that can be used as the
        destination for rendering.
*/
struct FrameBuffer
{
    uint32_t m_width, m_height;
    // access color_buffer using uint8_t*
    std::unique_ptr<Texture> color_buffer;
    // access color_buffer using float*
    std::unique_ptr<Texture> depth_buffer;

    FrameBuffer();
    ~FrameBuffer() = default;

    // attach a raw Texture* to FrameBuffer
    // The unique_ptr will take over the ownership
    bool attach_texture(attachment_type attachment, Texture *texture);

    // attach a std::unique_ptr<Texture> to FrameBuffer
    // The unique_ptr will take over the ownership
    bool attach_texture(attachment_type attachment, std::unique_ptr<Texture> &&texture);

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
    static void set_clear_color(float red, float green, float blue, float alpha);

    /*
        \brief Uses preset values to clear all buffers in the framebuffer.
            Each pixel of the color buffer will be cleared using the value previously
            set via the set_clear_color() function. For depth buffers, a fixed value of
            1 will be used to clear each pixel. If framebuffer is a null pointer, the
            function does nothing.

        \param framebuffer Pointer to the framebuffer to clear.
    */
    void clear();
};