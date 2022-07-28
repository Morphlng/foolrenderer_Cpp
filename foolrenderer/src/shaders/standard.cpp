#include "shaders/standard.h"

#define TEXCOORD 0
#define WORLD_SPACE_POSITION 0
#define WORLD_SPACE_NORMAL 1
#define WORLD_SPACE_TANGENT 2
#define WORLD_SPACE_BITANGENT 3
#define LIGHT_SPACE_POSITION 4

struct material_parameter {
    vec3 normal;  // In tangent space.
    vec3 base_color;
    float metallic;
    float roughness;
    float reflectance;
};

static inline float shadow(ShaderContext* input, const standard_uniform* uniform) {
    vec3 position = *input->shader_context_vec3(LIGHT_SPACE_POSITION);
    float current_depth = position.z;
    float bias = 0.005f;  // Slove shadow acne.
    float closest_depth = uniform->shadow_map->sample(position.to2D()).r;
    float visibility = current_depth - bias > closest_depth ? 0.0f : 1.0f;
    return visibility;
}

// Process user input of material properties into a form that is convenient for
// the shader to use.
static inline void compute_material_parameter(material_parameter* param, const standard_uniform* uniform,
    vec2 texcoord) {
    vec3 normal = uniform->normal_map->sample(texcoord).to3D();
    normal = normal * 2.0f - 1.0f;
    param->normal = normal;
    vec3 base_color = uniform->base_color_map->sample(texcoord).to3D();
    base_color = uniform->base_color * base_color;
    param->base_color = base_color;
    float metallic = uniform->metallic_map->sample(texcoord).r;
    metallic *= uniform->metallic;
    param->metallic = metallic;
    float roughness = uniform->roughness_map->sample(texcoord).r;
    roughness *= uniform->roughness;
    param->roughness = roughness;
    param->reflectance = uniform->reflectance;
}

static inline float perceptual_roughness_to_a2(float perceptual_roughness) {
    // Prevent being zero, and prevent perceptual_oughness^4 from going out of
    // range of precision.
    perceptual_roughness = std::max(perceptual_roughness, 0.045f);
    float roughness = perceptual_roughness * perceptual_roughness;
    return roughness * roughness;
}

static inline matrix3x3 construct_tangent2world(ShaderContext* input) {
    vec3 t = input->shader_context_vec3(WORLD_SPACE_TANGENT)->normalize();
    vec3 b = input->shader_context_vec3(WORLD_SPACE_BITANGENT)->normalize();
    vec3 n = input->shader_context_vec3(WORLD_SPACE_NORMAL)->normalize();
    return matrix3x3(t, b, n);
}

static inline float pow5(float x) {
    float x2 = x * x;
    return x2 * x2 * x;
}

static inline vec3 f_schlick(vec3 f0, float l_dot_h) {
    // Schlick's approximation is defined as:
    // f_schlick = f0 + (1 - f0) * (1 - l_dot_h)^5
    // This is the optimized code after reducing vector operations.
    float f = pow5(1.0f - l_dot_h);
    return f0 * (1.0f - f) + f;
}

static inline float d_ggx(float a2, float n_dot_h) {
    float f = (n_dot_h * a2 - n_dot_h) * n_dot_h + 1.0;
    return a2 / (PI * f * f);
}

static inline float v_smith_ggx_correlated(float a2, float n_dot_l,
    float n_dot_v) {
    // Height correlated Smith-GGX formulation:
    // lambda_v = 0.5 * (-1 + sqrt(a2 + (1 - a2) * n_dot_l^2) / n_dot_l)
    // lambda_l = 0.5 * (-1 + sqrt(a2 + (1 - a2) * n_dot_v^2) / n_dot_v)
    // g_smith_ggx_correlated = 1 / (1 + lambda_v + lambda_l)
    // v_smith_ggx_correlated = g_smith_ggx_correlated /
    //                      (4.0f * n_dot_l * n_dot_v)
    // This is the optimized code.
    float lambda_v = n_dot_l * sqrtf((n_dot_v - a2 * n_dot_v) * n_dot_v + a2);
    float lambda_l = n_dot_v * sqrtf((n_dot_l - a2 * n_dot_l) * n_dot_l + a2);
    return 0.5 / (lambda_v + lambda_l);
}

static inline vec3 specular_lobe(float a2, vec3 f0, float n_dot_h,
    float n_dot_l, float n_dot_v,
    float l_dot_h) {
    // Using Cook-Torrance microfacet BRDF.
    vec3 f = f_schlick(f0, l_dot_h);
    float d = d_ggx(a2, n_dot_h);
    float v = v_smith_ggx_correlated(a2, n_dot_l, n_dot_v);
    return f * d * v;
}

static inline vec3 diffuse_lobe(vec3 diffuse_color) {
    // Using Lambertian BRDF.
    return diffuse_color * (1.0f / PI);
}

vec4 standard_vertex_shader(ShaderContext* output, const void* uniform, const void* vertex_attribute) {
    const standard_uniform* unif = (const standard_uniform*)uniform;
    const standard_vertex_attribute* attr = (const standard_vertex_attribute*)vertex_attribute;

    vec2* out_texcoord = output->shader_context_vec2(TEXCOORD);
    *out_texcoord = attr->texcoord;

    vec4 world_position = unif->local2world * attr->position.to4D(1.0f);
    vec3* out_position = output->shader_context_vec3(WORLD_SPACE_POSITION);
    *out_position = world_position.to3D();

    vec3* out_normal = output->shader_context_vec3(WORLD_SPACE_NORMAL);
    *out_normal = unif->local2world_normal * attr->normal;

    vec3* out_tangent = output->shader_context_vec3(WORLD_SPACE_TANGENT);
    *out_tangent = unif->local2world_direction * attr->tangent.to3D();

    vec3* out_bitangent = output->shader_context_vec3(WORLD_SPACE_BITANGENT);
    *out_bitangent = out_normal->cross(*out_tangent) * attr->tangent.w;

    vec4 light_space_position = unif->world2light * world_position;
    vec3* out_light_space_position = output->shader_context_vec3(LIGHT_SPACE_POSITION);
    // When calculating directional light shadows, the view2clip matrix
    // contained in local2light is an orthogonal matrix, the w component is
    // always equal to 1.0f, so no need for homogeneous division.
    *out_light_space_position = light_space_position.to3D();

    return unif->world2clip * world_position;
}

vec4 standard_fragment_shader(ShaderContext* input, const void* uniform) {
    vec2 texcoord = *input->shader_context_vec2(TEXCOORD);
    vec3 position = *input->shader_context_vec3(WORLD_SPACE_POSITION);
    const standard_uniform* unif = (const standard_uniform*)uniform;
    vec3 camera_position = unif->camera_position;
    vec3 light_direction = unif->light_direction;
    vec3 illuminance = unif->illuminance;
    vec3 ambient_luminance = unif->ambient_luminance;

    material_parameter material;
    compute_material_parameter(&material, unif, texcoord);

    vec3 diffuse_color = material.base_color * (1.0f - material.metallic);
    float dielectric_f0 = 0.16f * material.reflectance * material.reflectance *
        (1.0f - material.metallic);
    vec3 conductor_f0 = material.base_color * material.metallic;
    vec3 f0 = conductor_f0 + dielectric_f0;
    float a2 = perceptual_roughness_to_a2(material.roughness);
    matrix3x3 tangent2world = construct_tangent2world(input);
    // Normalized normal, in world space.
    vec3 normal = tangent2world * material.normal;
    // Normalized vector from the fragment to the camera, in world space.
    vec3 view = (camera_position - position).normalize();
    // Normalized halfway vector between the light direction and the view
    // direction, in world space.
    vec3 halfway = (view + light_direction).normalize();

    float n_dot_v = std::max(normal.dot(view), 1e-4f); // Avoid artifact.
    float n_dot_l = std::max(normal.dot(light_direction), 0.0f);
    float n_dot_h = std::max(normal.dot(halfway), 0.0f);
    float l_dot_h = std::max(light_direction.dot(halfway), 0.0f);

    float visibility = shadow(input, unif);
    vec3 fr = specular_lobe(a2, f0, n_dot_h, n_dot_l, n_dot_v, l_dot_h);
    vec3 fd = diffuse_lobe(diffuse_color);
    // According to the ambient lighting is uniform:
    // ambient_illuminance = PI * ambient_luminance
    // fd = diffuse_color / PI
    // ambient_output = fd * ambient_illuminance
    //                = diffuse_color * ambient_luminance
    vec3 ambient_output = diffuse_color * ambient_luminance;
    vec3 output = (fr + fd) * illuminance;
    output = output * n_dot_l;
    output = output * visibility;
    output = output + ambient_output;
    return output.to4D(1.0f);
}
