#pragma once

#include "shader_context.h"
#include "texture.h"
#include "rmatrix.h"

// The basic shader implements Blinn-Phong reflection model with Phong shading.
// References:
// https://en.wikipedia.org/wiki/Blinn%E2%80%93Phong_reflection_model
// https://en.wikipedia.org/wiki/Phong_shading

struct basic_uniform {
    matrix4x4 local2view;
    matrix4x4 view2clip;
    matrix3x3 loacl2view_direction;
    matrix3x3 local2view_normal;
    // In light space, each component of position should be in [0,1].
    matrix4x4 local2light;

    // Parameters of the directional light.
    vec3 light_direction;  // Normalized light direction in view space.
    vec3 light_color;
    vec3 ambient_color;
    Texture* shadow_map;

    // Parameters of the material.
    vec3 ambient_reflectance;
    vec3 diffuse_reflectance;
    vec3 specular_reflectance;
    float shininess;
    Texture* diffuse_map;
    Texture* normal_map;
};

struct basic_vertex_attribute {
    vec3 position;
    vec3 normal;
    vec4 tangent;
    vec2 texcoord;
};

vec4 basic_vertex_shader(ShaderContext* output, const void* uniform, const void* vertex_attribute);

vec4 basic_fragment_shader(ShaderContext* input, const void* uniform);