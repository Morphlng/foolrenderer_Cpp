#include "shaders/basic.h"
#include "texture.h"

#define TEXCOORD 0
#define VIEW_SPACE_POSITION 0
#define LIGHT_SPACE_POSITION 1
#define VIEW_SPACE_NORMAL 2
#define VIEW_SPACE_TANGENT 3
#define VIEW_SPACE_BITANGENT 4

static float shadow_calculation(const Texture* shadow_map,
    const vec3 light_space_positon) {
    float visibility = 1.0f;
    if (shadow_map) {
        float current_depth = light_space_positon.z;
        float bias = 0.005f;  // Slove shadow acne.
        float closest_depth = shadow_map->sample(light_space_positon.to2D()).r;
        visibility = current_depth - bias > closest_depth ? 0.1f : 1.0f;
    }
    return visibility;
}

vec4 basic_vertex_shader(ShaderContext* output, const void* uniform, const void* vertex_attribute) {
    const basic_uniform* unif = (const basic_uniform*)uniform;
    const basic_vertex_attribute* attr = (const basic_vertex_attribute*)vertex_attribute;

    vec2* out_texcoord = output->shader_context_vec2(TEXCOORD);
    *out_texcoord = attr->texcoord;

    vec4 view_space_position = unif->local2view * attr->position.to4D(1.0f);
    vec3* out_position = output->shader_context_vec3(VIEW_SPACE_POSITION);
    *out_position = view_space_position.to3D();

    vec3* out_light_space_position = output->shader_context_vec3(LIGHT_SPACE_POSITION);
    vec4 light_space_position = unif->local2light * attr->position.to4D(1.0f);

    // When calculating directional light shadows, the view2clip matrix
    // contained in local2light is an orthogonal matrix, the w component is
    // always equal to 1.0f, so perspective division is not required.
    *out_light_space_position = light_space_position.to3D();

    // Calculate t,b,n vectors.
    vec3* out_normal = output->shader_context_vec3(VIEW_SPACE_NORMAL);
    *out_normal = unif->local2view_normal * attr->normal;

    vec3* out_tangent = output->shader_context_vec3(VIEW_SPACE_TANGENT);
    *out_tangent = unif->loacl2view_direction * attr->tangent.to3D();

    vec3* out_bitangent = output->shader_context_vec3(VIEW_SPACE_BITANGENT);
    *out_bitangent = out_normal->cross(*out_tangent) * attr->tangent.w;

    return unif->view2clip * view_space_position;
}

vec4 basic_fragment_shader(ShaderContext* input, const void* uniform) {
    const basic_uniform* unif = (const basic_uniform*)uniform;
    vec2 texcoord = *input->shader_context_vec2(TEXCOORD);

    // Get the normal in tangent space.
    vec3 normal = unif->normal_map->sample(texcoord).to3D();
    normal = normal * 2.0f - 1.0f;

    // Transform the normal from tangent space to view space
    vec3 t = input->shader_context_vec3(VIEW_SPACE_TANGENT)->normalize();
    vec3 b = input->shader_context_vec3(VIEW_SPACE_BITANGENT)->normalize();
    vec3 n = input->shader_context_vec3(VIEW_SPACE_NORMAL)->normalize();
    normal = matrix3x3(t, b, n) * normal;

    // Ambient lighting.
    vec3 ambient_lighting = unif->ambient_color * unif->ambient_reflectance;

    // Diffuse lighting.
    float n_dot_l = normal.dot(unif->light_direction);
    float diffuse_intensity = std::max(0.0f, n_dot_l);
    vec3 diffuse_lighting = unif->light_color * diffuse_intensity;

    diffuse_lighting = diffuse_lighting * unif->diffuse_reflectance;

    // Specular lighting.
    vec3 specular_lighting = VEC3_ZERO;
    if (n_dot_l > 0.0f) {
        // Because in view space,the camera position is always at (0,0,0), so
        // the calculation of the view direction is simplified.
        vec3 postion = *input->shader_context_vec3(VIEW_SPACE_POSITION);
        vec3 view_direction = postion * -1.0f;
        view_direction = view_direction.normalize();
        // Calculate the halfway vector between the light direction and the view
        // direction.
        vec3 halfway = view_direction + unif->light_direction;
        halfway = halfway.normalize();
        float n_dot_h = normal.dot(halfway);
        float specular_intensity = powf(std::max(0.0f, n_dot_h), unif->shininess);
        specular_lighting = unif->light_color * specular_intensity;
        specular_lighting = specular_lighting * unif->specular_reflectance;
    }

    // Add shadow.
    vec3 light_space_position = *input->shader_context_vec3(LIGHT_SPACE_POSITION);
    float visibility = shadow_calculation(unif->shadow_map, light_space_position);
    diffuse_lighting = diffuse_lighting * visibility;
    specular_lighting = specular_lighting * visibility;

    vec4 texture_color = unif->diffuse_map->sample(texcoord);
    vec3 fragment_color = ambient_lighting + diffuse_lighting;
    fragment_color = fragment_color * texture_color.to3D();
    fragment_color = fragment_color + specular_lighting;
    return fragment_color.to4D(1.0f);
}