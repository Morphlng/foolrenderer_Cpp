#include <memory>
#include <string>

#include "graphics/framebuffer.h"
#include "graphics/rasterizer.h"
#include "graphics/texture.h"
#include "rmath/rmatrix.h"
#include "rmath/rvector.h"
#include "shaders/shadow_casting.h"
#include "shaders/standard.h"
#include "utility/image.h"
#include "utility/mesh.h"

#define SHADOW_MAP_WIDTH 1024
#define SHADOW_MAP_HEIGHT 1024
#define IMAGE_WIDTH 1024
#define IMAGE_HEIGHT 1024

struct Model {
    std::unique_ptr<Mesh> mesh;
    std::unique_ptr<Texture> base_color_map;
    std::unique_ptr<Texture> normal_map;
    std::unique_ptr<Texture> metallic_map;
    std::unique_ptr<Texture> roughness_map;
};

static vec3 light_direction{1.0f, 4.0f, -1.0f};
static vec3 camera_position{-2.0f, 4.5f, 2.0f};
static vec3 camera_target{0.0f, 0.4f, 0.0f};

static FrameBuffer shadow_framebuffer;
static Texture *shadow_map{
    nullptr};  // this is only a pointer, does not have ownership
static FrameBuffer framebuffer;
static Texture *color_buffer{nullptr};
static Texture *depth_buffer{nullptr};

static matrix4x4 light_world2clip;

static void initialize_rendering() {
    shadow_framebuffer.attach_texture(
        attachment_type::DEPTH_ATTACHMENT,
        std::make_unique<Texture>(texture_format::TEXTURE_FORMAT_DEPTH_FLOAT,
                                  SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT));
    shadow_map = shadow_framebuffer.depth_buffer.get();

    framebuffer.attach_texture(
        attachment_type::COLOR_ATTACHMENT,
        std::make_unique<Texture>(texture_format::TEXTURE_FORMAT_SRGB8_A8,
                                  IMAGE_WIDTH, IMAGE_HEIGHT));
    framebuffer.attach_texture(
        attachment_type::DEPTH_ATTACHMENT,
        std::make_unique<Texture>(texture_format::TEXTURE_FORMAT_DEPTH_FLOAT,
                                  IMAGE_WIDTH, IMAGE_HEIGHT));

    color_buffer = framebuffer.color_buffer.get();
    depth_buffer = framebuffer.depth_buffer.get();
}

static void render_shadow_map(const Model *model) {
    set_viewport(0, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);
    set_vertex_shader(shadow_casting_vertex_shader);
    set_fragment_shader(shadow_casting_fragment_shader);
    shadow_framebuffer.clear();

    shadow_casting_uniform uniform;
    vec3 light_position = light_direction.normalize() * 5.0f;
    matrix4x4 world2view =
        matrix_t::look_at(light_position, VEC3_ZERO, vec3{0.0f, 1.0f, 0.0f});
    matrix4x4 view2clip = matrix_t::orthographic(1.5f, 1.5f, 0.1f, 6.0f);
    light_world2clip = view2clip * world2view;
    // No rotation, scaling, or translation of the model, so the local2clip
    // matrix is the world2clip matrix.
    uniform.local2clip = light_world2clip;

    const Mesh *mesh = model->mesh.get();
    uint32_t triangle_count = mesh->triangle_count;
    for (size_t t = 0; t < triangle_count; t++) {
        shadow_casting_vertex_attribute attributes[3];
        const void *attribute_ptrs[3];
        for (uint32_t v = 0; v < 3; v++) {
            attributes[v].position = mesh->get_mesh_position(t, v);
            attribute_ptrs[v] = attributes + v;
        }
        draw_triangle(&shadow_framebuffer, &uniform, attribute_ptrs);
    }
}

static void render_model(const Model *model) {
    set_viewport(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT);
    set_vertex_shader(standard_vertex_shader);
    set_fragment_shader(standard_fragment_shader);
    FrameBuffer::set_clear_color(0.49f, 0.33f, 0.41f, 1.0f);
    framebuffer.clear();

    standard_uniform uniform;
    uniform.local2world = MATRIX4x4_IDENTITY;
    matrix4x4 world2view = matrix_t::look_at(camera_position, camera_target,
                                             vec3{0.0f, 1.0f, 0.0f});
    matrix4x4 view2clip = matrix_t::orthographic(2.0f, 2.0f, 0.1f, 10.0f);
    uniform.world2clip = view2clip * world2view;
    uniform.local2world_direction = uniform.local2world.to_3x3();
    // There is no non-uniform scaling so the normal transformation matrix is
    // the direction transformation matrix.
    uniform.local2world_normal = uniform.local2world_direction;
    uniform.camera_position = camera_position;
    uniform.light_direction = light_direction.normalize();
    uniform.illuminance = vec3{4.0f, 4.0f, 4.0f};
    // Remap each component of position from [-1, 1] to [0, 1].
    matrix4x4 scale_bias = {{0.5f, 0.0f, 0.0f, 0.5f},
                            {0.0f, 0.5f, 0.0f, 0.5f},
                            {0.0f, 0.0f, 0.5f, 0.5f},
                            {0.0f, 0.0f, 0.0f, 1.0f},
                            false};
    uniform.world2light = scale_bias * light_world2clip;
    uniform.shadow_map = shadow_map;
    uniform.ambient_luminance = vec3{1.0f, 0.5f, 0.8f};
    uniform.normal_map = model->normal_map.get();
    uniform.base_color = VEC3_ONE;
    uniform.base_color_map = model->base_color_map.get();
    uniform.metallic = 1.0f;
    uniform.metallic_map = model->metallic_map.get();
    uniform.roughness = 1.0f;
    uniform.roughness_map = model->roughness_map.get();
    uniform.reflectance = 0.5f;  // Common dielectric surfaces F0.

    const Mesh *mesh = model->mesh.get();
    uint32_t triangle_count = mesh->triangle_count;
    for (size_t t = 0; t < triangle_count; t++) {
        standard_vertex_attribute attributes[3];
        const void *attribute_ptrs[3];
        for (uint32_t v = 0; v < 3; v++) {
            attributes[v].position = mesh->get_mesh_position(t, v);
            attributes[v].normal = mesh->get_mesh_normal(t, v);
            attributes[v].tangent = mesh->get_mesh_tangent(t, v);
            attributes[v].texcoord = mesh->get_mesh_texcoord(t, v);
            attribute_ptrs[v] = attributes + v;
        }
        draw_triangle(&framebuffer, &uniform, attribute_ptrs);
    }
}

int main() {
    auto base_path = std::string("..\\assets\\cut_fish\\");
    auto model_path = base_path + "cut_fish.obj";
    auto base_color_map_path = base_path + "base_color.tga";
    auto normal_map_path = base_path + "normal.tga";
    auto metallic_map_path = base_path + "metallic.tga";
    auto roughness_map_path = base_path + "roughness.tga";

    Model model;
    model.mesh = std::make_unique<Mesh>(model_path);
    model.base_color_map = load_image(base_color_map_path, true);
    model.normal_map = load_image(normal_map_path, false);
    model.metallic_map = load_image(metallic_map_path, false);
    model.roughness_map = load_image(roughness_map_path, false);

    initialize_rendering();
    render_shadow_map(&model);
    render_model(&model);
    save_image(color_buffer, "output.tga", false);

    return 0;
}