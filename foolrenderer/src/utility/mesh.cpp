#include "mesh.h"

#define VERTEX_EQUAL(a, b) \
    ((a)->p == (b)->p && (a)->t == (b)->t && (a)->n == (b)->n)

// Put vertex at the end of vertex_set and return its index in vertex_set, If
// the vertex has been put into vertex_set, the index is returned directly.
static uint32_t put_vertex(fastObjIndex vertex_set[], uint32_t& vertex_set_size,
    const fastObjIndex* vertex) {
    // The current solution is stupid but simple to implement...
    // This function can be optimized using a hash map.
    for (uint32_t i = 0; i < vertex_set_size; i++) {
        fastObjIndex* v = vertex_set + i;
        if (VERTEX_EQUAL(v, vertex)) {
            return i;
        }
    }
    vertex_set[vertex_set_size] = *vertex;
    ++(vertex_set_size);
    return vertex_set_size - 1;
}

Mesh::Mesh(std::string_view filename)
{
    load_model(filename);
}

void Mesh::load_model(std::string_view filename)
{
    // use std::unique_ptr for exception safe deletion
    auto fastObjDeleter = [](fastObjMesh* data) { if (data)fast_obj_destroy(data); };
    std::unique_ptr<fastObjMesh, decltype(fastObjDeleter)> data(fast_obj_read(filename.data()), fastObjDeleter);

    do {
        if (!data) {
            break;
        }
        if (!set_vertex_attributes(data.get())) {
            break;
        }
        if (!set_diffuse_texture_name(data.get())) {
            break;
        }
        if (!normals) {
            if (!compute_normals()) {
                break;
            }
        }
        if (!compute_tangents()) {
            break;
        }

        return;

    } while (0);

    clean_up();
}

void Mesh::clean_up()
{
    positions.reset();
    texcoords.reset();
    normals.reset();
    tangents.reset();
    indices.reset();
    diffuse_texture_path.clear();
    vertex_count = 0;
    triangle_count = 0;
}

vec3 Mesh::get_mesh_position(uint32_t triangle_index, uint32_t vertex_index) const
{
    if (triangle_index >= triangle_count || vertex_index > 2) {
        return VEC3_ZERO;
    }
    else {
        uint32_t index = indices[triangle_index * 3 + vertex_index];
        return positions[index];
    }
}

vec2 Mesh::get_mesh_texcoord(uint32_t triangle_index, uint32_t vertex_index) const
{
    if (triangle_index >= triangle_count || vertex_index > 2 || !texcoords) {
        return VEC2_ZERO;
    }
    else {
        uint32_t index = indices[triangle_index * 3 + vertex_index];
        return texcoords[index];
    }
}

vec3 Mesh::get_mesh_normal(uint32_t triangle_index, uint32_t vertex_index) const
{
    if (triangle_index >= triangle_count || vertex_index > 2 || !normals) {
        return VEC3_ZERO;
    }
    else {
        uint32_t index = indices[triangle_index * 3 + vertex_index];
        return normals[index];
    }
}

vec4 Mesh::get_mesh_tangent(uint32_t triangle_index, uint32_t vertex_index) const
{
    if (triangle_index >= triangle_count || vertex_index > 2 || !tangents) {
        return VEC4_ZERO;
    }
    else {
        uint32_t index = indices[triangle_index * 3 + vertex_index];
        return tangents[index];
    }
}

bool Mesh::set_vertex_attributes(const fastObjMesh* data)
{
    uint32_t index_count = 0;
    for (unsigned int f = 0; f < data->face_count; f++) {
        unsigned int face_vertices = data->face_vertices[f];
        if (face_vertices != 0 && face_vertices != 3) {
            // Failed to load mesh if mesh contains non-triangular faces. Faces
            // with 0 vertices can be ignored directly.
            return false;
        }
        index_count += face_vertices;
    }

    indices.reset(new uint32_t[index_count]);
    if (!indices) {
        return false;
    }

    std::unique_ptr<fastObjIndex[]> vertex_set = std::make_unique<fastObjIndex[]>(index_count);
    uint32_t vertex_set_size = 0;
    // Check if the model file contains texcoord or normal data as they are
    // optional.
    bool has_texcoords = false;
    bool has_normals = false;

    for (uint32_t i = 0; i < index_count; i++) {
        const fastObjIndex* vertex = data->indices + i;
        uint32_t vertex_set_index =
            put_vertex(vertex_set.get(), vertex_set_size, vertex);
        indices[i] = vertex_set_index;
        // A mesh is considered to contain texcoord or normal data as long as
        // one vertex contains a valid texcoord or normal index.
        unsigned int ti = vertex->t;
        has_texcoords = has_texcoords || (ti > 0 && ti < data->texcoord_count);
        unsigned int ni = vertex->n;
        has_normals = has_normals || (ni > 0 && ni < data->normal_count);
    }

    positions.reset(new vec3[vertex_set_size]);
    texcoords.reset(has_texcoords ? new vec2[vertex_set_size] : nullptr);
    normals.reset(has_texcoords ? new vec3[vertex_set_size] : nullptr);

    if (!positions || (has_texcoords && !texcoords) ||
        (has_normals && !normals)) {
        indices.reset();
        positions.reset();
        texcoords.reset();
        normals.reset();
        return false;
    }

    for (uint32_t i = 0; i < vertex_set_size; i++) {
        float* src;
        fastObjIndex* vertex = vertex_set.get() + i;

        // Set positions.
        if (vertex->p < data->position_count) {
            src = data->positions + vertex->p * 3;
        }
        else {
            // index out of bounds, use dummy data at index 0.
            src = data->positions;
        }
        memcpy((positions.get() + i)->elements, data->positions + vertex->p * 3,
            sizeof(float) * 3);
        // Set texcoords.
        if (has_texcoords) {
            if (vertex->t < data->texcoord_count) {
                src = data->texcoords + vertex->t * 2;
            }
            else {
                src = data->texcoords;
            }
            memcpy((texcoords.get() + i)->elements, data->texcoords + vertex->t * 2,
                sizeof(float) * 2);
        }
        // Set normals.
        if (has_normals) {
            if (vertex->n < data->normal_count) {
                src = data->normals + vertex->n * 3;
            }
            else {
                src = data->normals;
            }
            memcpy((normals.get() + i)->elements, data->normals + vertex->n * 3,
                sizeof(float) * 3);
            // Normal data in .obj files may not be normalized.
            normals[i] = normals[i].normalize();
        }
    }

    vertex_count = vertex_set_size;
    triangle_count = index_count / 3;
    return true;
}

bool Mesh::set_diffuse_texture_name(const fastObjMesh* data)
{
    diffuse_texture_path.clear();
    if (data->material_count == 0) {
        return true;
    }

    char* texture_path = data->materials->map_Kd.path;
    if (texture_path == nullptr) {
        return true;
    }

    diffuse_texture_path.assign(texture_path);
    if (diffuse_texture_path.length() <= 1)
        diffuse_texture_path.clear();

    return true;
}

bool Mesh::compute_normals()
{
    normals.reset(new vec3[vertex_count]);
    if (!normals) {
        return false;
    }

    for (uint32_t v = 0; v < vertex_count; v++) {
        normals[v] = VEC3_ZERO;
    }

    for (uint32_t t = 0; t < triangle_count; t++) {
        // For calculating surface normals, refer to:
        // https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal
        uint32_t index_0 = indices[t * 3];
        uint32_t index_1 = indices[t * 3 + 1];
        uint32_t index_2 = indices[t * 3 + 2];
        const vec3& p0 = positions[index_0];
        const vec3& p1 = positions[index_1];
        const vec3& p2 = positions[index_2];
        vec3 u = p1 - p0;
        vec3 v = p2 - p0;
        // Vertices are stored in counterclockwise order by default in .obj
        // files. And the foolrenderer uses a right-handed coordinate system. So
        // use "n = u X v" to calculate the normal.
        vec3 n = u.cross(v);
        // Add the surface normal of the triangle to the normals already present
        // on the three vertices of the triangle. Note that the triangle surface
        // normal is not normalized, its magnitude is twice the area of the
        // triangle, so that the normal direction of a triangle with a larger
        // area has a larger contribution to the normal direction of adjacent
        // vertices.
        normals[index_0] = normals[index_0] + n;
        normals[index_1] = normals[index_1] + n;
        normals[index_2] = normals[index_2] + n;
    }
    // Normalize the normals of all vertices to get the average result.
    for (uint32_t v = 0; v < vertex_count; v++) {
        normals[v] = normals[v].normalize();
    }

    return true;
}

bool Mesh::compute_tangents()
{
    if (!normals || !texcoords) {
        tangents.reset();
        return false;
    }

    tangents.reset(new vec4[vertex_count]);
    if (!tangents) {
        return false;
    }

    // Temporary array for tangents and bitangents and initialize to zero
    // vector.
    std::unique_ptr<vec3[]> tmp_tangents = std::make_unique<vec3[]>(vertex_count);
    std::unique_ptr<vec3[]> tmp_bitangents = std::make_unique<vec3[]>(vertex_count);
    for (uint32_t v = 0; v < vertex_count; v++) {
        tmp_tangents[v] = VEC3_ZERO;
        tmp_bitangents[v] = VEC3_ZERO;
    }
    // This function use Lengyel¡¯s method, for more details refer to:
    // http://www.terathon.com/code/tangent.html
    for (uint32_t t = 0; t < triangle_count; t++) {
        uint32_t index_0 = indices[t * 3];
        uint32_t index_1 = indices[t * 3 + 1];
        uint32_t index_2 = indices[t * 3 + 2];
        const vec3& p0 = positions[index_0];
        const vec3& p1 = positions[index_1];
        const vec3& p2 = positions[index_2];
        const vec2& w0 = texcoords[index_0];
        const vec2& w1 = texcoords[index_1];
        const vec2& w2 = texcoords[index_2];

        vec3 e1 = p1 - p0;
        vec3 e2 = p2 - p0;
        float x1 = w1.u - w0.u;
        float x2 = w2.u - w0.u;
        float y1 = w1.v - w0.v;
        float y2 = w2.v - w0.v;

        float d = x1 * y2 - x2 * y1;
        vec3 tangent, bitangent;
        if (d == 0.0f) {
            tangent = VEC3_ZERO;
            bitangent = VEC3_ZERO;
        }
        else {
            float r = 1.0f / d;
            tangent = e1 * y2 - e2 * y1;
            tangent = tangent * r;
            bitangent = e2 * x1 - e1 * x2;
            bitangent = bitangent * r;
        }
        tmp_tangents[index_0] = tmp_tangents[index_0] + tangent;
        tmp_tangents[index_1] = tmp_tangents[index_1] + tangent;
        tmp_tangents[index_2] = tmp_tangents[index_2] + tangent;
        tmp_bitangents[index_0] = tmp_bitangents[index_0] + bitangent;
        tmp_bitangents[index_1] = tmp_bitangents[index_1] + bitangent;
        tmp_bitangents[index_2] = tmp_bitangents[index_2] + bitangent;
    }

    for (uint32_t v = 0; v < vertex_count; v++) {
        vec3& t = tmp_tangents[v];
        const vec3& b = tmp_bitangents[v];
        const vec3& n = normals[v];
        // Gram-Schmidt orthogonalize.
        t = t - n * n.dot(t);
        t = t.normalize();

        vec4& tangent = tangents[v];
        tangent.x = t.x;
        tangent.y = t.y;
        tangent.z = t.z;
        tangent.w = n.cross(t).dot(b) < 0.0f ? -1.0f : 1.0f;
    }
    return true;
}
