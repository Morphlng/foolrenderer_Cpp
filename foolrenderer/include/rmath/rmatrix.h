#pragma once
#include "rvector.h"
#include <iostream>

/*
	Matrix class, required row == column
	Use SFINAE to enable different function for 3x3 or 4x4 matrix
*/
template <size_t row, size_t column, std::enable_if_t<row == column, bool>>
struct matrix;

// name alias for 3x3 matrix
using matrix3x3 = matrix<3, 3, true>;
// name alias for 4x4 matrix
using matrix4x4 = matrix<4, 4, true>;
// name alias for default matrix, used for static function
using matrix_t = matrix<3, 3, true>;

#define MATRIX3x3_ZERO (matrix3x3{{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}})
#define MATRIX3x3_IDENTITY (matrix3x3{{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}})
#define MATRIX4x4_ZERO (matrix4x4{{0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}})
#define MATRIX4x4_IDENTITY (matrix4x4{{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}})

template <size_t row, size_t column, std::enable_if_t<row == column, bool> = true>
struct matrix
{
	float elements[row][column];

	matrix()
	{
		std::fill(elements[0], elements[0] + row * column, 0.0f);
	}

	~matrix() = default;

	// ------------3x3 matrix------------

	/*
		Construct a 3x3 matrix using given 3 vector
		Default is Column first sequence, set is_column_first = false to enable Row_first
	*/
	template <size_t U = row, std::enable_if_t<U == 3, bool> = true>
	matrix(vec3 v1, vec3 v2, vec3 v3, bool is_column_first = true)
	{
		if (is_column_first)
		{
			elements[0][0] = v1.x, elements[0][1] = v2.x, elements[0][2] = v3.x;
			elements[1][0] = v1.y, elements[1][1] = v2.y, elements[1][2] = v3.y;
			elements[2][0] = v1.z, elements[2][1] = v2.z, elements[2][2] = v3.z;
		}
		else
		{
			std::copy(std::begin(v1.elements), std::end(v1.elements), elements[0]);
			std::copy(std::begin(v2.elements), std::end(v2.elements), elements[1]);
			std::copy(std::begin(v3.elements), std::end(v3.elements), elements[2]);
		}
	}
	template <size_t U = row, std::enable_if_t<U == 3, bool> = true>
	vec3 operator*(const vec3 &v) const
	{
		auto result = VEC3_ZERO;

		for (int i = 0; i < row; i++)
		{
			for (int j = 0; j < column; j++)
			{
				result.elements[i] += elements[i][j] * v.elements[j];
			}
		}

		return result;
	}

	template <size_t U = row, std::enable_if_t<U == 3, bool> = true>
	matrix3x3 operator*(const matrix3x3 &right) const
	{
		auto result = MATRIX3x3_ZERO;

		for (int v = 0; v < 3; v++)
		{
			for (int i = 0; i < row; i++)
			{
				for (int j = 0; j < column; j++)
				{
					result.elements[i][v] += elements[i][j] * right.elements[j][v];
				}
			}
		}

		return result;
	}

	// ------------4x4 matrix------------

	/*
		Construct a 4x4 matrix using given 4 vector
		Default is Column first sequence, set is_column_first = false to enable Row_first
	*/
	template <size_t U = row, std::enable_if_t<U == 4, bool> = true>
	matrix(vec4 v1, vec4 v2, vec4 v3, vec4 v4, bool is_column_first = true)
	{
		if (is_column_first)
		{
			elements[0][0] = v1.x, elements[0][1] = v2.x, elements[0][2] = v3.x, elements[0][3] = v4.x;
			elements[1][0] = v1.y, elements[1][1] = v2.y, elements[1][2] = v3.y, elements[1][3] = v4.y;
			elements[2][0] = v1.z, elements[2][1] = v2.z, elements[2][2] = v3.z, elements[2][3] = v4.z;
			elements[3][0] = v1.w, elements[3][1] = v2.w, elements[3][2] = v3.w, elements[3][3] = v4.w;
		}
		else
		{
			std::copy(std::begin(v1.elements), std::end(v1.elements), elements[0]);
			std::copy(std::begin(v2.elements), std::end(v2.elements), elements[1]);
			std::copy(std::begin(v3.elements), std::end(v3.elements), elements[2]);
			std::copy(std::begin(v4.elements), std::end(v4.elements), elements[3]);
		}
	}

	/*
		\brief Constructs a matrix3x3 from the upper-left of matrix4x4.
	*/
	template <size_t U = row, std::enable_if_t<U == 4, bool> = true>
	matrix3x3 to_3x3() const
	{
		return matrix<3, 3>{
			vec3{elements[0][0], elements[0][1], elements[0][2]},
			vec3{elements[1][0], elements[1][1], elements[1][2]},
			vec3{elements[2][0], elements[2][1], elements[2][2]}, false};
	}

	template <size_t U = row, std::enable_if_t<U == 4, bool> = true>
	vec4 operator*(const vec4 &v) const
	{
		auto result = VEC4_ZERO;

		for (int i = 0; i < row; i++)
		{
			for (int j = 0; j < column; j++)
			{
				result.elements[i] += elements[i][j] * v.elements[j];
			}
		}

		return result;
	}

	template <size_t U = row, std::enable_if_t<U == 4, bool> = true>
	matrix4x4 operator*(const matrix4x4 &right) const
	{
		auto result = MATRIX4x4_ZERO;

		for (int v = 0; v < 4; v++)
		{
			for (int i = 0; i < row; i++)
			{
				for (int j = 0; j < column; j++)
				{
					result.elements[i][v] += elements[i][j] * right.elements[j][v];
				}
			}
		}

		return result;
	}

	/*
		\brief If the matrix is invertible, gets the inverse of the matrix.
				Returns a zero matrix otherwise.
		\return The inverse of the matrix.
	*/
	template <size_t U = row, std::enable_if_t<U == 4, bool> = true>
	matrix4x4 inverse() const
	{
		const float a11 = elements[0][0], a12 = elements[0][1],
					a13 = elements[0][2], a14 = elements[0][3];
		const float a21 = elements[1][0], a22 = elements[1][1],
					a23 = elements[1][2], a24 = elements[1][3];
		const float a31 = elements[2][0], a32 = elements[2][1],
					a33 = elements[2][2], a34 = elements[2][3];
		const float a41 = elements[3][0], a42 = elements[3][1],
					a43 = elements[3][2], a44 = elements[3][3];

		// Uses the adjugate of the matrix to calculates the inverse.
		matrix4x4 adj;
		adj.elements[0][0] = a22 * a33 * a44 + a23 * a34 * a42 + a24 * a32 * a43 -
							 a24 * a33 * a42 - a23 * a32 * a44 - a22 * a34 * a43;
		adj.elements[0][1] = -a12 * a33 * a44 - a13 * a34 * a42 - a14 * a32 * a43 +
							 a14 * a33 * a42 + a13 * a32 * a44 + a12 * a34 * a43;
		adj.elements[0][2] = a12 * a23 * a44 + a13 * a24 * a42 + a14 * a22 * a43 -
							 a14 * a23 * a42 - a13 * a22 * a44 - a12 * a24 * a43;
		adj.elements[0][3] = -a12 * a23 * a34 - a13 * a24 * a32 - a14 * a22 * a33 +
							 a14 * a23 * a32 + a13 * a22 * a34 + a12 * a24 * a33;

		adj.elements[1][0] = -a21 * a33 * a44 - a23 * a34 * a41 - a24 * a31 * a43 +
							 a24 * a33 * a41 + a23 * a31 * a44 + a21 * a34 * a43;
		adj.elements[1][1] = a11 * a33 * a44 + a13 * a34 * a41 + a14 * a31 * a43 -
							 a14 * a33 * a41 - a13 * a31 * a44 - a11 * a34 * a43;
		adj.elements[1][2] = -a11 * a23 * a44 - a13 * a24 * a41 - a14 * a21 * a43 +
							 a14 * a23 * a41 + a13 * a21 * a44 + a11 * a24 * a43;
		adj.elements[1][3] = a11 * a23 * a34 + a13 * a24 * a31 + a14 * a21 * a33 -
							 a14 * a23 * a31 - a13 * a21 * a34 - a11 * a24 * a33;

		adj.elements[2][0] = a21 * a32 * a44 + a22 * a34 * a41 + a24 * a31 * a42 -
							 a24 * a32 * a41 - a22 * a31 * a44 - a21 * a34 * a42;
		adj.elements[2][1] = -a11 * a32 * a44 - a12 * a34 * a41 - a14 * a31 * a42 +
							 a14 * a32 * a41 + a12 * a31 * a44 + a11 * a34 * a42;
		adj.elements[2][2] = a11 * a22 * a44 + a12 * a24 * a41 + a14 * a21 * a42 -
							 a14 * a22 * a41 - a12 * a21 * a44 - a11 * a24 * a42;
		adj.elements[2][3] = -a11 * a22 * a34 - a12 * a24 * a31 - a14 * a21 * a32 +
							 a14 * a22 * a31 + a12 * a21 * a34 + a11 * a24 * a32;

		adj.elements[3][0] = -a21 * a32 * a43 - a22 * a33 * a41 - a23 * a31 * a42 +
							 a23 * a32 * a41 + a22 * a31 * a43 + a21 * a33 * a42;
		adj.elements[3][1] = a11 * a32 * a43 + a12 * a33 * a41 + a13 * a31 * a42 -
							 a13 * a32 * a41 - a12 * a31 * a43 - a11 * a33 * a42;
		adj.elements[3][2] = -a11 * a22 * a43 - a12 * a23 * a41 - a13 * a21 * a42 +
							 a13 * a22 * a41 + a12 * a21 * a43 + a11 * a23 * a42;
		adj.elements[3][3] = a11 * a22 * a33 + a12 * a23 * a31 + a13 * a21 * a32 -
							 a13 * a22 * a31 - a12 * a21 * a33 - a11 * a23 * a32;

		float determinant = a11 * adj.elements[0][0] + a21 * adj.elements[0][1] +
							a31 * adj.elements[0][2] + a41 * adj.elements[0][3];

		if (determinant == 0.0f)
		{
			// The matrix is not invertible.
			return MATRIX4x4_ZERO;
		}
		return adj * (1.0f / determinant);
	}

	// ------------general utility------------

	matrix<row, column> operator*(float scalar) const
	{
		auto out = *this;

		for (int i = 0; i < row; i++)
		{
			for (int j = 0; j < column; j++)
			{
				out.elements[i][j] *= scalar;
			}
		}

		return out;
	}

	matrix<row, column> transpose() const
	{
		matrix<row, column> out;
		for (int i = 0; i < row; i++)
		{
			for (int j = 0; j < column; j++)
			{
				out.elements[j][i] = elements[i][j];
			}
		}

		return out;
	}

	friend std::ostream &operator<<(std::ostream &out, const matrix<row, column> &mat)
	{
		for (int i = 0; i < row; i++)
		{
			for (int j = 0; j < column; j++)
			{
				out << mat.elements[i][j] << ' ';
			}
			out << '\n';
		}

		return out;
	}

	// ------------static function------------

	/*
		\brief Constructs a scaling matrix.
		\param scaling Specifys the scaling factor on the x, y and z axis.
		\return The scaling matrix.
	*/
	static matrix4x4 scale(vec3 scaling)
	{
		matrix4x4 result = MATRIX4x4_IDENTITY;
		result.elements[0][0] = scaling.x;
		result.elements[1][1] = scaling.y;
		result.elements[2][2] = scaling.z;
		return result;
	}

	/*
		\brief Constructs a translation matrix.
		\param translation Specifys the translation on the x, y and z axis.
		\return The translation matrix.
	*/
	static matrix4x4 translate(vec3 translation)
	{
		matrix4x4 result = MATRIX4x4_IDENTITY;
		result.elements[0][3] = translation.x;
		result.elements[1][3] = translation.y;
		result.elements[2][3] = translation.z;
		return result;
	}

	/*
		\brief Constructs a rotation matrix along the x axis.
		\param angle The angle of rotation along the x axis, in radians.
		\return The rotation matrix.
	*/
	static matrix4x4 rotate_x(float angle)
	{
		float c = cosf(angle);
		float s = sinf(angle);

		matrix4x4 result = MATRIX4x4_IDENTITY;
		result.elements[1][1] = c;
		result.elements[1][2] = -s;
		result.elements[2][1] = s;
		result.elements[2][2] = c;

		return result;
	}

	/*
		\brief Constructs a rotation matrix along the y axis.
		\param angle The angle of rotation along the y axis, in radians.
		\return The rotation matrix.
	*/
	static matrix4x4 rotate_y(float angle)
	{
		float c = cosf(angle);
		float s = sinf(angle);

		matrix4x4 result = MATRIX4x4_IDENTITY;
		result.elements[0][0] = c;
		result.elements[0][2] = s;
		result.elements[2][0] = -s;
		result.elements[2][2] = c;

		return result;
	}

	/*
		\brief Constructs a rotation matrix along the z axis.
		\param angle The angle of rotation along the z axis, in radians.
		\return The rotation matrix.
	*/
	static matrix4x4 rotate_z(float angle)
	{
		float c = cosf(angle);
		float s = sinf(angle);

		matrix4x4 result = MATRIX4x4_IDENTITY;
		result.elements[0][0] = c;
		result.elements[0][1] = -s;
		result.elements[1][0] = s;
		result.elements[1][1] = c;

		return result;
	}

	/*
		\brief Constructs a rotation matrix about an arbitrary vector.
		\param angle The angle of rotation, in radians.
		\param about The vector.
		\return The rotation matrix.
	*/
	static matrix4x4 rotate(float angle, vec3 about)
	{
		if (about.x == 1.0f && about.y == 0.0f && about.z == 0.0f)
		{
			return rotate_x(angle);
		}
		if (about.x == 0.0f && about.y == 1.0f && about.z == 0.0f)
		{
			return rotate_y(angle);
		}
		if (about.x == 0.0f && about.y == 0.0f && about.z == 1.0f)
		{
			return rotate_z(angle);
		}

		matrix4x4 result = MATRIX4x4_IDENTITY;
		float c = cosf(angle);
		float s = sinf(angle);
		about = about.normalize();
		float nc = 1 - c;
		float xy = about.x * about.y;
		float yz = about.y * about.z;
		float zx = about.z * about.x;
		float xs = about.x * s;
		float ys = about.y * s;
		float zs = about.z * s;

		result.elements[0][0] = about.x * about.x * nc + c;
		result.elements[0][1] = xy * nc - zs;
		result.elements[0][2] = zx * nc + ys;

		result.elements[1][0] = xy * nc + zs;
		result.elements[1][1] = about.y * about.y * nc + c;
		result.elements[1][2] = yz * nc - xs;

		result.elements[2][0] = zx * nc - ys;
		result.elements[2][1] = yz * nc + xs;
		result.elements[2][2] = about.z * about.z * nc + c;

		return result;
	}

	/*
		\brief Constructs a view matrix.
			The view matrix is used to transform the world
			space vertices to the view space.
		\param from The position of the camera point.
		\param to The position of the target point.
		\param up The direction of the up vector.
		\return The view matrix.
	*/
	static matrix4x4 look_at(vec3 from, vec3 to, vec3 up)
	{
		// In foolrenderer, world space and view space are right-handed coordinate
		// systems (matches OpenGL convention), so the direction of z_axis is
		// opposite to the direction in which the camera points to the target.
		vec3 z_axis = (from - to).normalize();
		vec3 x_axis = up.cross(z_axis).normalize();
		vec3 y_axis = z_axis.cross(x_axis);
		matrix4x4 result = MATRIX4x4_IDENTITY;

		result.elements[0][0] = x_axis.x;
		result.elements[0][1] = x_axis.y;
		result.elements[0][2] = x_axis.z;

		result.elements[1][0] = y_axis.x;
		result.elements[1][1] = y_axis.y;
		result.elements[1][2] = y_axis.z;

		result.elements[2][0] = z_axis.x;
		result.elements[2][1] = z_axis.y;
		result.elements[2][2] = z_axis.z;

		result.elements[0][3] = -x_axis.dot(from);
		result.elements[1][3] = -y_axis.dot(from);
		result.elements[2][3] = -z_axis.dot(from);

		return result;
	}

	/*
		\brief Constructs a perspective projection matrix, follow OpenGL convention.
		\param fov The vertical field of view in radians.
		\param aspect The aspect ration (width divided by height).
		\param near The distance to the near depth clipping plane.
		\param far The distance to the far depth clipping plane.
		\return The perspective projection matrix.
	*/
	static matrix4x4 perspective(float fov, float aspect, float near, float far)
	{
		matrix4x4 result = MATRIX4x4_ZERO;
		float fn = far - near;
		result.elements[1][1] = 1.0f / tanf(fov / 2.0f);
		result.elements[0][0] = result.elements[1][1] / aspect;
		result.elements[2][2] = (-far - near) / fn;
		result.elements[2][3] = (-2.0f * far * near) / fn;
		result.elements[3][2] = -1.0f;
		return result;
	}

	/*
		\brief Constructs an orthogonal projection matrix, follow OpenGL convention.
		\param right Coordinate of the right clipping plane .
		\param top Coordinates of the top clipping plane.
		\param near The distance to the near depth clipping plane.
		\param far The distance to the far depth clipping plane.
		\return The orthogonal projection matrix.
	*/
	static matrix4x4 orthographic(float right, float top, float near, float far)
	{
		matrix4x4 result = MATRIX4x4_IDENTITY;
		float fn = far - near;
		result.elements[0][0] = 1.0f / right;
		result.elements[1][1] = 1.0f / top;
		result.elements[2][2] = -2.0f / fn;
		result.elements[2][3] = (-near - far) / fn;
		return result;
	}
};