#include "rmath/rvector.h"

// -------------vec2 implementation-------------
vec3 vec2::to3D(float z) const
{
	return vec3{x, y, z};
}

vec2 vec2::operator+(const vec2 &rhs) const
{
	return vec2{x + rhs.x, y + rhs.y};
}

vec2 vec2::operator+(float scalar) const
{
	return vec2{x + scalar, y + scalar};
}

vec2 vec2::operator-(const vec2 &rhs) const
{
	return vec2{x - rhs.x, y - rhs.y};
}

vec2 vec2::operator-(float scalar) const
{
	return vec2{x - scalar, y - scalar};
}

vec2 vec2::operator*(const vec2 &rhs) const
{
	return vec2{x - rhs.x, y - rhs.y};
}

vec2 vec2::operator*(float scalar) const
{
	return vec2{x * scalar, y * scalar};
}

vec2 vec2::operator/(const vec2 &rhs) const
{
	return vec2{x - rhs.x, y - rhs.y};
}

vec2 vec2::operator/(float scalar) const
{
	return vec2{x * scalar, y * scalar};
}

float vec2::dot(const vec2 &rhs) const
{
	return x * rhs.x + y * rhs.y;
}

float vec2::magnitude() const
{
	return sqrtf(dot(*this));
}

float vec2::magnitude_squared() const
{
	return dot(*this);
}

vec2 vec2::normalize() const
{
	float square_magnitude = magnitude_squared();
	if (square_magnitude == 0.0f)
	{
		return VEC2_ZERO;
	}
	if (fabsf(square_magnitude - 1.0f) < SMALL_ABSOLUTE_FLOAT)
	{
		return *this;
	}
	return this->operator*(1.0f / sqrtf(square_magnitude));
}

vec2 vec2::vec2_lerp(const vec2 &rhs, float t) const
{
	float _x = lerp(x, rhs.x, t);
	float _y = lerp(y, rhs.y, t);
	return vec2{_x, _y};
}

// -------------vec3 implementation-------------
vec2 vec3::to2D() const
{
	return vec2{x, y};
}

vec4 vec3::to4D(float w) const
{
	return vec4{x, y, z, w};
}

vec3 vec3::operator+(const vec3 &rhs) const
{
	return vec3{x + rhs.x, y + rhs.y, z + rhs.z};
}

vec3 vec3::operator+(float scalar) const
{
	return vec3{x + scalar, y + scalar, z + scalar};
}

vec3 vec3::operator-(const vec3 &rhs) const
{
	return vec3{x - rhs.x, y - rhs.y, z - rhs.z};
}

vec3 vec3::operator-(float scalar) const
{
	return vec3{x - scalar, y - scalar, z - scalar};
}

vec3 vec3::operator*(const vec3 &rhs) const
{
	return vec3{x * rhs.x, y * rhs.y, z * rhs.z};
}

vec3 vec3::operator*(float scalar) const
{
	return vec3{x * scalar, y * scalar, z * scalar};
}

vec3 vec3::operator/(const vec3 &rhs) const
{
	return vec3{x / rhs.x, y / rhs.y, z / rhs.z};
}

vec3 vec3::operator/(float scalar) const
{
	return vec3{x / scalar, y / scalar, z / scalar};
}

float vec3::dot(const vec3 &rhs) const
{
	return x * rhs.x + y * rhs.y + z * rhs.z;
}

vec3 vec3::cross(const vec3 &rhs) const
{
	return vec3{
		y * rhs.z - z * rhs.y,
		z * rhs.x - x * rhs.z,
		x * rhs.y - y * rhs.x};
}

float vec3::magnitude() const
{
	return sqrtf(dot(*this));
}

float vec3::magnitude_squared() const
{
	return dot(*this);
}

vec3 vec3::normalize() const
{
	float square_magnitude = magnitude_squared();
	if (square_magnitude == 0.0f)
	{
		return VEC3_ZERO;
	}
	if (fabsf(square_magnitude - 1.0f) < SMALL_ABSOLUTE_FLOAT)
	{
		return (*this);
	}
	return this->operator*(1.0f / sqrtf(square_magnitude));
}

vec3 vec3::vec3_lerp(const vec3 &b, float t) const
{
	float _x = lerp(x, b.x, t);
	float _y = lerp(y, b.y, t);
	float _z = lerp(z, b.z, t);
	return vec3{_x, _y, _z};
}

// -------------vec4 implementation-------------
vec2 vec4::to2D() const
{
	return vec2{x, y};
}

vec3 vec4::to3D() const
{
	return vec3{x, y, z};
}

vec4 vec4::operator+(const vec4 &rhs) const
{
	return vec4{x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w};
}

vec4 vec4::operator+(float scalar) const
{
	return vec4{x + scalar, y + scalar, z + scalar, w + scalar};
}

vec4 vec4::operator-(const vec4 &rhs) const
{
	return vec4{x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w};
}

vec4 vec4::operator-(float scalar) const
{
	return vec4{x - scalar, y - scalar, z - scalar, w - scalar};
}

vec4 vec4::operator*(const vec4 &rhs) const
{
	return vec4{x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w};
}

vec4 vec4::operator*(float scalar) const
{
	return vec4{x * scalar, y * scalar, z * scalar, w * scalar};
}

vec4 vec4::operator/(const vec4 &rhs) const
{
	return vec4{x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w};
}

vec4 vec4::operator/(float scalar) const
{
	return vec4{x / scalar, y / scalar, z / scalar, w / scalar};
}

float vec4::dot(const vec4 &rhs) const
{
	return x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w;
}

float vec4::magnitude() const
{
	return sqrtf(dot(*this));
}

float vec4::magnitude_squared() const
{
	return dot(*this);
}

vec4 vec4::normalize() const
{
	float square_magnitude = magnitude_squared();
	if (square_magnitude == 0.0f)
	{
		return VEC4_ZERO;
	}
	if (fabsf(square_magnitude - 1.0f) < SMALL_ABSOLUTE_FLOAT)
	{
		return (*this);
	}
	return this->operator*(1.0f / sqrtf(square_magnitude));
}

vec4 vec4::vec4_lerp(const vec4 &b, float t) const
{
	float _x = lerp(x, b.x, t);
	float _y = lerp(y, b.y, t);
	float _z = lerp(z, b.z, t);
	float _w = lerp(w, b.w, t);
	return vec4{_x, _y, _z, _w};
}