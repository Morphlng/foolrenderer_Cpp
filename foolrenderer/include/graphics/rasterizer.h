#pragma once

#include "framebuffer.h"
#include "shader_context.h"
#include "texture.h"
#include "rvector.h"

///
/// \brief Pointer to vertex shader.
///
/// The uniform is constant that can be accessed in vertex shader and fragment
/// shader. It usually used to pass transformation matrices, light directions,
/// and material properties to the shader.
///
/// The vertex attribute represents the data elements of a vertex that the
/// vertex shader needs to process��such as vertex positions, normal and texture
/// coordinates.
///
/// The vertex shader returns the clip space position of the vertex, the clip
/// space should follow the OpenGL convention, using the left-handed coordinate
/// system, the near plane is at z=-1, and the far plane is at z=1.
///
/// Any other output produced needs to be saved in the shader context. These
/// output values will be interpolated across the face of the rendered
/// triangles, and the value of each pixel will be passed as input to the
/// fragment shader.
///
using vertex_shader = vec4(*)(ShaderContext* output, const void* uniform, const void* vertex_attribute);

///
/// \brief Pointer to fragment shader.
///
/// The uniform is constant that can be accessed in vertex shader and fragment
/// shader. It usually used to pass transformation matrices, light directions,
/// and material properties to the shader.
///
/// The input stores the values assigned by the vertex shader, which are
/// interpolated before being received by the fragment shader.
///
/// The fragment shader returns the color value.
///
using fragment_shader = vec4(*)(ShaderContext* input, const void* uniform);

///
/// \brief Set the viewport parameters.
///
/// Viewport describe a view port by its bottom-left coordinate, width and
/// height in pixels.
///
/// \param left Left coordinate in pixel.
/// \param bottom Bottom coordinate in pixel.
/// \param width Width in pixel.
/// \param height Height in pixel.
///
void set_viewport(int left, int bottom, uint32_t width, uint32_t height);

void set_vertex_shader(vertex_shader shader);

void set_fragment_shader(fragment_shader shader);

///
/// \brief Render triangle.
///
/// Before calling this function, need to ensure that the rendering state has
/// been set through the set_viewport(), set_vertex_shader() and
/// set_fragment_shader() functions.
///
/// When the triangle finally appears on the screen after all transformations,
/// if the vertex connection sequence is counterclockwise, the triangle is
/// treated as front face. This function only draws front-facing triangles.
///
/// Always assumes the shader's output is in linear RGB color space. So if the
/// color buffer attached to the framebuffer is sRGB encoded, convert the output
/// from linear RGB to sRGB. If there is no color buffer attached, the fragment
/// color result is discarded. If the framebuffer is not attached with a depth
/// buffer, the depth test is not performed.
///
/// \param framebuffer Buffer for saving rendering results.
/// \param uniform Contains constants that can be accessed in the vertex shader
///                and fragment shader.
/// \param vertex_attributes An array containing vertex attributes, with a
///                          length of 3.
///
void draw_triangle(FrameBuffer* framebuffer, const void* uniform,
    const void* const vertex_attributes[]);