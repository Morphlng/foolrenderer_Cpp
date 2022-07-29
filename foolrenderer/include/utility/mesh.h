#pragma once

#include "rmath/rvector.h"
#include "fast_obj.h"
#include <memory>
#include <string>
#include <string_view>

///
/// The mesh is made of triangles, each triangle is defined by three vertex
/// indices. For example, a cube mesh has 12 triangles, then the indices array
/// length should be 36, with each value indicating which vertex to use. The
/// first three elements in the indices array are the indices of the vertices
/// that make up the triangle; the second three elements make up another
/// triangle, and so on.
///
/// For every vertex there can be a vertex position, texture coordinate
/// (texcoord), normal and tangent. These data are collectively called vertex
/// attributes. Vertex attributes other than position are optional.
//
/// All vertex attributes (if present) are stored in separate arrays of the same
/// size. For example, if a mesh has 100 vertices, and each vertex has position
/// and texcoord, then the mesh should have positions and texcoords arrays, each
/// being 100 in size. Data for i-th vertex is at index "i" in each array.
/// Pointers to other vertex attributes should point to null pointers.
///
/// If the mesh has no diffuse texture associated with it, diffuse_texture_path
/// points to a null pointer.
///
struct Mesh
{
    std::unique_ptr<vec3[]> positions;
    std::unique_ptr<vec2[]> texcoords;
    ///
    /// \brief The normals of the mesh.
    ///
    /// Normals are unit vectors.
    ///
    std::unique_ptr<vec3[]> normals;
    ///
    /// \brief The tangents of the mesh.
    ///
    /// Tangents are mostly used in normal mapping. A tangent is a unit vector
    /// that follows mesh surface along horizontal (u) texture direction. The
    /// type of tangents is vector4, with x, y, z components defining the
    /// vector, the w component is used to determine the direction of the
    /// bitangent when calculating the bitangent.
    ///
    /// In foolrenderer, calculate the bitangent by taking a cross product
    /// between the normal and the tangent, and multiplying the result by the w
    /// component of the tangent. Therefor, w should always be 1 or -1, if
    /// tangent exists.
    ///
    std::unique_ptr<vec4[]> tangents;
    std::unique_ptr<uint32_t[]> indices;
    std::string diffuse_texture_path;
    uint32_t vertex_count;
    uint32_t triangle_count;

    Mesh() = default;
    Mesh(std::string_view filename);
    void load_model(std::string_view filename);

    ~Mesh() = default;
    void clean_up();

    vec3 get_mesh_position(uint32_t triangle_index, uint32_t vertex_index) const;
    vec2 get_mesh_texcoord(uint32_t triangle_index, uint32_t vertex_index) const;
    vec3 get_mesh_normal(uint32_t triangle_index, uint32_t vertex_index) const;
    vec4 get_mesh_tangent(uint32_t triangle_index, uint32_t vertex_index) const;

private:
    // Returns false if failed, otherwise returns true.
    bool set_vertex_attributes(const fastObjMesh *data);

    // Returns false if failed, otherwise returns true.
    bool set_diffuse_texture_name(const fastObjMesh *data);

    // Calculates the average unit-length normal vector for each vertex in the mesh.
    //
    // Returns false if failed, otherwise returns true.
    bool compute_normals();

    // Calculates the average unit-length tangent vector for each vertex of the mesh
    // from the normals and texcoords. If the mesh does not have normals or
    // texcoords, the mesh's tangents will point to a null pointer.
    //
    // Returns false if failed, otherwise returns true.
    bool compute_tangents();
};