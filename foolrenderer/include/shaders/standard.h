#pragma once

#include "shader_context.h"
#include "texture.h"
#include "rmatrix.h"
#include "rvector.h"

// The standard shader implements a physically based rendering (PBR) material
// model. And use the specific implementation of the Google Filament project:
// https://google.github.io/filament/Filament.html
//
// This model is composed of a diffuse term and a specular term. Can be used to
// render common opaque metallic/non-metallic objects.
struct standard_uniform {
    matrix4x4 local2world;
    matrix4x4 world2clip;
    matrix3x3 local2world_direction;
    matrix3x3 local2world_normal;
    // Camera position in world space.
    vec3 camera_position;
    // Normalized directional light direction in world space.
    vec3 light_direction;
    // Directional light illuminance.
    vec3 illuminance;
    // Transform vertex positions from world space to directional light¡®s light
    // space.
    matrix4x4 world2light;
    // Directional light shadow map
    Texture* shadow_map;
    // Suppose the ambient lighting is uniform from all directions.
    vec3 ambient_luminance;

    ////////////////////////////////////////////////////////////////////////////
    //
    // Material parameters.
    //
    ////////////////////////////////////////////////////////////////////////////
    Texture* normal_map;
    // Diffuse albedo for non-metallic surfaces and specular color for metallic
    // surfaces, should be in linear color space. A specular color reference
    // table for metals can be found in the Filament documentation:
    // https://google.github.io/filament/Filament.html#table_fnormalmetals
    vec3 base_color;
    Texture* base_color_map;
    // Whether a surface appears to be dielectric (0.0) or conductor (1.0).
    float metallic;
    Texture* metallic_map;
    // Perceived smoothness (0.0) or roughness (1.0) of a surface.
    float roughness;
    Texture* roughness_map;
    // Fresnel reflectance at normal incidence for dielectric surfaces, not
    // useful for conductor surfaces. A reference table of reflectance for
    // dielectric can be found in the Filament documentation:
    // https://google.github.io/filament/Filament.html#table_commonmatreflectance
    float reflectance;
};

struct standard_vertex_attribute {
    vec3 position;
    vec3 normal;
    vec4 tangent;
    vec2 texcoord;
};

vec4 standard_vertex_shader(ShaderContext* output, const void* uniform, const void* vertex_attribute);

vec4 standard_fragment_shader(ShaderContext* input, const void* uniform);