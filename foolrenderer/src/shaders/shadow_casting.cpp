#include "shaders/shadow_casting.h"

vec4 shadow_casting_vertex_shader(ShaderContext* output, const void* uniform, const void* vertex_attribute) {
    (void)output;
    const shadow_casting_uniform* unif = (const shadow_casting_uniform*)uniform;
    const shadow_casting_vertex_attribute* attr = (const shadow_casting_vertex_attribute*)vertex_attribute;

    vec4 position = attr->position.to4D(1.0f);
    return unif->local2clip * position;
}

vec4 shadow_casting_fragment_shader(ShaderContext* input, const void* uniform) {
    (void)input;
    (void)uniform;
    return VEC4_ZERO;
}