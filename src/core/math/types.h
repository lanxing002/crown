/*
 * Copyright (c) 2012-2024 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/types.h"
#include <array>

/// @defgroup Math Math
/// @ingroup Core
namespace crown
{
/// @addtogroup Math
/// @{

struct Vector2
{
	f32 x, y;
};

struct Vector3
{
	f32 x, y, z;
};

struct Vector4
{
	f32 x, y, z, w;
};

/// RGBA color.
typedef Vector4 Color4;

struct Quaternion
{
	f32 x, y, z, w;
};

struct Matrix3x3
{
	Vector3 x, y, z;
};

struct Matrix4x4
{
	Vector4 x, y, z, t;
};

struct AABB
{
	Vector3 min{ 3.402823466e+38F , 3.402823466e+38F , 3.402823466e+38F };
	Vector3 max{ 1.175494351e-38F , 1.175494351e-38F , 1.175494351e-38F };
};

struct OBB
{
	Matrix4x4 tm;
	Vector3 half_extents;
};

/// 3D Plane.
/// The form is ax + by + cz + d = 0
/// where: d = vector3::dot(n, p)
struct Plane3
{
	Vector3 n;
	f32 d;
};

struct Frustum
{
	std::array<Plane3, 6> planes;
};

struct Sphere
{
	Vector3 c;
	f32 r;
};

/// @}

} // namespace crown
