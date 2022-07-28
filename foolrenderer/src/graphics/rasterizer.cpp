#include "rasterizer.h"
#include "color.h"
#include <cstdint>

static struct {
	int left, bottom;
	uint32_t width, height;
} viewport;

struct vertex {
	ShaderContext context;
	vec4 position;
	vec2 screen_space_position;
	float depth;

	/* 
		The inverse of the w component of the vertex position in the clip space.
		Will be used for perspective correct interpolation.
	*/
	float inverse_w;

	/*
		The vertex position should be in clippig space.
		Returns true if the vertex needs to be clipped, otherwise returns false.
	*/
	bool clipping_test() {
		float w = this->position.w;
		for (int c = 0; c < 3; c++) {
			float component = this->position.elements[c];
			if (component < -w || component > w) {
				return true;
			}
		}
		return false;
	}

	// Transform vertex position from clip space to normalized device coordinates
	// (NDC).
	void perspective_division() {
		float inv_w = 1.0f / position.w;
		this->inverse_w = inv_w;
		position.x *= inv_w;
		position.y *= inv_w;
		position.z *= inv_w;
		position.w = 1.0f;
	}

	/*
		Transform the x and y components of position from the NDC to the screen
		space, transform the value range of the z component from [-1, 1] to [0, 1].
	*/
	void viewport_transform() {
		screen_space_position.x =
			(position.x + 1.0f) * 0.5f * viewport.width + viewport.left;
		screen_space_position.y =
			(position.y + 1.0f) * 0.5f * viewport.height + viewport.bottom;
		this->depth = (position.z + 1.0f) * 0.5f;
	}
};

struct bounding_box {
	vec2 min, max;

	void update(const vertex& vtx) {
		auto& position = vtx.screen_space_position;
		min.x = std::min(min.x, position.x);
		min.y = std::min(min.y, position.y);
		max.x = std::max(max.x, position.x);
		max.y = std::max(max.y, position.y);
	}
};

static vertex_shader vs = nullptr;
static fragment_shader fs = nullptr;

// Framebuffer data.
static uint32_t framebuffer_width = 0;
static uint32_t framebuffer_height = 0;
static uint8_t* color_buffer = nullptr;
static bool is_srgb_encoding = false;
static float* depth_buffer = nullptr;

void parse_framebuffer(FrameBuffer& framebuffer)
{
	framebuffer_width = framebuffer.m_width;
	framebuffer_height = framebuffer.m_height;

	auto& color_attachment = framebuffer.color_buffer;
	if (!color_attachment) {
		color_buffer = nullptr;
		is_srgb_encoding = false;
	}
	else {
		color_buffer = color_attachment->get_pixels();
		is_srgb_encoding =
			(color_attachment->m_format == texture_format::TEXTURE_FORMAT_SRGB8_A8) ?
			true : false;
	}

	auto& depth_attachment = framebuffer.depth_buffer;
	if (!depth_attachment) {
		depth_buffer = nullptr;
	}
	else {
		depth_buffer = (float*)depth_attachment->get_pixels();
	}
}

// Computes the determinant of a 2x2 matrix composed of vectors (c-a) and (b-a).
// The result can be interpreted as the signed area of a parallelogram with the
// two vectors as sides. At the same time, the sign of the area can be used to
// determine the left-right relationship between the two vectors.
float edge_function(const vec2& a, const vec2& b, const vec2& c) {
	return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
}

// Returns true if the fragment is hidden. If the fragment is not hidden, return
// false. If depth_test is a null pointer, skip the depth test and always return
// false.
bool depth_test(uint32_t x, uint32_t y, const struct vertex vertices[], const float barycentric[]) {
	if (depth_buffer == nullptr) {
		return false;
	}
	// Interpolate depth, for more details refer to the OpenGL specification
	// section 3.6.1 equation 3.10:
	// https://www.khronos.org/registry/OpenGL/specs/gl/glspec33.core.pdf
	// For the purpose of reducing computational overhead, the calculated depth
	// value is in the screen space, and the depth value in this space is not
	// linear. Although it is enough for depth testing.
	float new_depth = barycentric[0] * vertices[0].depth +
		barycentric[1] * vertices[1].depth +
		barycentric[2] * vertices[2].depth;
	float* depth = depth_buffer + (y * framebuffer_width + x);
	bool is_hidden = new_depth > *depth;
	if (!is_hidden) {
		*depth = new_depth;
	}
	return is_hidden;
}

// For the principle of perspective correct interpolation vertex variables,
// refer to:
// https://www.comp.nus.edu.sg/~lowkl/publications/lowk_persp_interp_techrep.pdf
//
// The OpenGL specification section 3.6.1 provides the same calculation method,
// refer to equation 3.9:
// https://www.khronos.org/registry/OpenGL/specs/gl/glspec33.core.pdf
// This interpolation method is suitable for both perspective projection and
// orthogonal projection.
void interpolate_variables(float* result, const float* const sources[], size_t component_count, float inverse_denominator, const float bc_over_w[]) {
	const float* s0 = sources[0];
	const float* s1 = sources[1];
	const float* s2 = sources[2];
	for (size_t i = 0; i < component_count; i++) {
		float numerator =
			s0[i] * bc_over_w[0] + s1[i] * bc_over_w[1] + s2[i] * bc_over_w[2];
		result[i] = numerator * inverse_denominator;
	}
}

#define INTERPOLATION_HELPER(type, component_count)                      \
    do {                                                                 \
        variable_count = vertices[0].context.type##_variable_count;      \
        for (int8_t i = 0; i < variable_count; i++) {                    \
            int8_t index = vertices[0].context.type##_index_queue[i];    \
            variables[0] =                                               \
                (float *)(vertices[0].context.type##_variables + index); \
            variables[1] =                                               \
                (float *)(vertices[1].context.type##_variables + index); \
            variables[2] =                                               \
                (float *)(vertices[2].context.type##_variables + index); \
            float *interpolate_result =                                  \
                (float *)result->shader_context_##type(index);           \
            interpolate_variables(interpolate_result, variables,         \
                                  component_count, inverse_denominator,  \
                                  bc_over_w);                            \
        }                                                                \
    } while (0)

void set_fragment_shader_input(ShaderContext* result, const vertex vertices[], const float barycentric[]) {
	float bc_over_w[3];
	for (int i = 0; i < 3; i++) {
		bc_over_w[i] = barycentric[i] * vertices[i].inverse_w;
	}
	float inverse_denominator =
		1.0f / (bc_over_w[0] + bc_over_w[1] + bc_over_w[2]);

	const float* variables[3];
	int8_t variable_count;
	INTERPOLATION_HELPER(float, 1);
	INTERPOLATION_HELPER(vec2, 2);
	INTERPOLATION_HELPER(vec3, 3);
	INTERPOLATION_HELPER(vec4, 4);
}

void write_color(uint8_t* pixel, vec4 color) {
	color.r = clamp01(color.r);
	color.g = clamp01(color.g);
	color.b = clamp01(color.b);
	color.a = clamp01(color.a);
	if (is_srgb_encoding) {
		// Perform gamma correction if the color buffer to be written is sRGB
		// encoded.
		color.r = convert_to_srgb_color(color.r);
		color.g = convert_to_srgb_color(color.g);
		color.b = convert_to_srgb_color(color.b);
	}
	pixel[0] = float_to_uint8(color.r);
	pixel[1] = float_to_uint8(color.g);
	pixel[2] = float_to_uint8(color.b);
	pixel[3] = float_to_uint8(color.a);
}

void set_viewport(int left, int bottom, uint32_t width, uint32_t height) {
	viewport.left = left;
	viewport.bottom = bottom;
	viewport.width = width;
	viewport.height = height;
}

void set_vertex_shader(vertex_shader shader) { vs = shader; }

void set_fragment_shader(fragment_shader shader) { fs = shader; }

// Using edge functions to raster triangles, refer to:
// https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/rasterization-stage
void draw_triangle(FrameBuffer* framebuffer, const void* uniform, const void* const vertex_attributes[]) {
	if (vs == nullptr || fs == nullptr || framebuffer == nullptr) {
		return;
	}
	parse_framebuffer(*framebuffer);
	vertex vertices[3];
	// The bounding box of the triangle.
	bounding_box bound = { {FLT_MAX, FLT_MAX}, {FLT_MIN, FLT_MIN} };
	for (int i = 0; i < 3; i++) {
		auto& vtx = vertices[i];
		vtx.context.clear();
		vtx.position = vs(&vtx.context, uniform, vertex_attributes[i]);
		// Perform a rough clipping test, if at least one vertex is outside the
		// viewing volume, the entire triangle will be discarded.
		if (vtx.clipping_test()) {
			return;
		}
		vtx.perspective_division();
		vtx.viewport_transform();
		bound.update(vtx);
	}
	// Compute the area of the triangle multiplied by 2.
	float area = edge_function(vertices[0].screen_space_position,
		vertices[1].screen_space_position,
		vertices[2].screen_space_position);
	if (area >= 0) {
		// If the area is 0, it means this is a degenerate triangle. If the area
		// is positive, the triangle with clockwise winding.
		// In both cases, the triangle does not need to be drawn.
		return;
	}
	float inverse_area = 1 / area;

	// Traverse find the pixels covered by the triangle. If found, compute the
	// barycentric coordinates of the point in the triangle.
	// No need to traverses pixels outside the screen.
	uint32_t x_min = clamp<uint32_t>(floorf(bound.min.x), 0, framebuffer_width - 1);
	uint32_t y_min = clamp<uint32_t>(floorf(bound.min.y), 0, framebuffer_height - 1);
	uint32_t x_max = clamp<uint32_t>(floorf(bound.max.x), 0, framebuffer_width - 1);
	uint32_t y_max = clamp<uint32_t>(floorf(bound.max.y), 0, framebuffer_height - 1);

	for (uint32_t y = y_min; y <= y_max; y++) {
		for (uint32_t x = x_min; x <= x_max; x++) {
			vec2 p { x, y };
			// The barycentric coordinates of p.
			float bc[3];
			// Note that this is not the final barycentric coordinates.
			bc[0] = edge_function(vertices[1].screen_space_position,
				vertices[2].screen_space_position, p);
			bc[1] = edge_function(vertices[2].screen_space_position,
				vertices[0].screen_space_position, p);
			bc[2] = edge_function(vertices[0].screen_space_position,
				vertices[1].screen_space_position, p);
			if (bc[0] > 0.0f || bc[1] > 0.0f || bc[2] > 0.0f) {
				// If any component of the barycentric coordinates is greater
				// than 0, it means that the pixel is outside the triangle.
				continue;
			}
			// Calculate the barycentric coordinates of point p.
			bc[0] *= inverse_area;
			bc[1] *= inverse_area;
			bc[2] *= inverse_area;

			if (depth_test(x, y, vertices, bc)) {
				continue;
			}
			ShaderContext input;
			input.clear();
			set_fragment_shader_input(&input, vertices, bc);
			vec4 fragment_color = fs(&input, uniform);
			if (color_buffer != nullptr) {
				uint8_t* pixel = color_buffer + (y * framebuffer_width + x) * 4;
				write_color(pixel, fragment_color);
			}
		}
	}
}
