#pragma once
#include "base_util.h"
#include <cmath>

struct vec2;
struct vec3;
struct vec4;

/* 
	Representing 2d space components
	You can access data through three ways:
	1. vec.x, vec.y as point
	2. vec.u, vec.v respectively
	3. vec.elements[index] as an array
*/
struct vec2
{
	union {
		struct {
			float x, y;
		};
		struct {
			float u, v;
		};
		float elements[2];
	};

	vec3 to3D(float z) const;

	vec2 operator+(const vec2& rhs) const;

	vec2 operator+(float scalar) const;

	vec2 operator-(const vec2& rhs) const;

	vec2 operator-(float scalar) const;

	vec2 operator*(const vec2& rhs) const;

	vec2 operator*(float scalar) const;

	vec2 operator/(const vec2& rhs) const;

	vec2 operator/(float scalar) const;

	float dot(const vec2& rhs) const;

	float magnitude() const;

	float magnitude_squared() const;

	vec2 normalize() const;

	vec2 vec2_lerp(const vec2& rhs, float t) const;
};

/*
	Representing 3d space components
	You can access data through three ways:
	1. vec.x, vec.y, vec.z as point
	2. vec.r, vec.g, vec.b as color
	3. vec.elements[index] as an array
*/
struct vec3
{
	union {
		struct {
			float x, y, z;
		};
		struct {
			float r, g, b;
		};
		float elements[3];
	};

	vec2 to2D() const;

	vec4 to4D(float w) const;

	vec3 operator+(const vec3& rhs) const;

	vec3 operator+(float scalar) const;

	vec3 operator-(const vec3& rhs) const;

	vec3 operator-(float scalar) const;

	vec3 operator*(const vec3& rhs) const;

	vec3 operator*(float scalar) const;

	vec3 operator/(const vec3& rhs) const;

	vec3 operator/(float scalar) const;

	float dot(const vec3& rhs) const;

	vec3 cross(const vec3& rhs) const;

	float magnitude() const;

	float magnitude_squared() const;

	vec3 normalize() const;

	vec3 vec3_lerp(const vec3& b, float t) const;
};

/*
	Representing 4d space components
	You can access data through three ways:
	1. vec.x, vec.y, vec.z, vec.w as point
	2. vec.r, vec.g, vec.b, vec.a as color with alpha channel
	3. vec.elements[index] as an array
*/
struct vec4
{
	union {
		struct {
			float x, y, z, w;
		};
		struct {
			float r, g, b, a;
		};
		float elements[4];
	};

	vec2 to2D() const;

	vec3 to3D() const;

	vec4 operator+(const vec4& rhs) const;

	vec4 operator+(float scalar) const;

	vec4 operator-(const vec4& rhs) const;

	vec4 operator-(float scalar) const;

	vec4 operator*(const vec4& rhs) const;

	vec4 operator*(float scalar) const;

	vec4 operator/(const vec4& rhs) const;

	vec4 operator/(float scalar) const;

	float dot(const vec4& rhs) const;

	float magnitude() const;

	float magnitude_squared() const;

	vec4 normalize() const;

	vec4 vec4_lerp(const vec4& b, float t) const;
};

#define VEC2_ZERO vec2{ 0.0f,0.0f }
#define VEC2_ONE vec2{ 1.0f,1.0f }
#define VEC3_ZERO vec3{ 0.0f,0.0f,0.0f }
#define VEC3_ONE vec3{ 1.0f,1.0f,1.0f }
#define VEC4_ZERO vec4{ 0.0f,0.0f,0.0f,0.0f }
#define VEC4_ONE vec4{ 1.0f,1.0f,1.0f,1.0f }