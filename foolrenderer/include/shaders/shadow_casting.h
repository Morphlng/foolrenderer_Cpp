#pragma once

#include "graphics/shader_context.h"
#include "rmath/rmatrix.h"
#include "rmath/rvector.h"

// The shadow casting shader is used to render shadow maps. For shadow mapping
// algorithm, refer to:
// https://en.wikipedia.org/wiki/Shadow_mapping

struct shadow_casting_uniform
{
    matrix4x4 local2clip;
};

struct shadow_casting_vertex_attribute
{
    vec3 position;
};

vec4 shadow_casting_vertex_shader(ShaderContext *output,
                                  const void *uniform,
                                  const void *vertex_attribute);

vec4 shadow_casting_fragment_shader(ShaderContext *input,
                                    const void *uniform);