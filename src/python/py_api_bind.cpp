#include "pybind11/pybind11.h"
#include "pybind11/operators.h"

#include "fmt/format.h"
#include "fmt/ranges.h"
#include "device/device.h"


namespace py = pybind11;
using namespace crown;


namespace py = pybind11;
using namespace crown;

#include "core/math/intersection.h"
#include "core/math/types.h"
#include "core/math/vector2.inl"
#include "core/math/vector3.inl"
#include "core/math/vector4.inl"

#include "core/math/aabb.inl"
#include "core/math/obb.inl"

auto format_as(const Vector2& v)
{
	return fmt::format("Vector2(x:{}, y:{})", v.x, v.y);
}

auto format_as(const Vector3& v)
{
	return fmt::format("Vector3(x:{}, y:{}, z:{})", v.x, v.y, v.z);
}

auto format_as(const Vector4& v)
{
	return fmt::format("Vector4(x:{}, y:{}, z:{}, w:{})", v.x, v.y, v.z, v.w);
}

auto format_as(const Quaternion& q)
{
	return fmt::format("Quaternion(x:{}, y:{}, z:{}, w:{})", q.x, q.y, q.z, q.w);
}

auto format_as(const Matrix3x3& m)
{
	return fmt::format("Matrix3x3(x:{}, y:{}, z:{})", m.x, m.y, m.z);
}

auto format_as(const Matrix4x4& m)
{
	return fmt::format("Matrix3x3(x:{}, y:{}, z:{}, t:{})", m.x, m.y, m.z, m.t);
}

auto format_as(const AABB& a)
{
	return fmt::format("AABB(min:{}, max:{})", a.min, a.max);
}

auto format_as(const OBB& o)
{
	return fmt::format("OBB(tm:{}, half_extents:{})", o.tm, o.half_extents);
}

auto format_as(const Plane3& p)
{
	return fmt::format("Plane3(n:{}, d:{})", p.n, p.d);
}

auto format_as(const Frustum& f)
{
	return fmt::format("Frustum({})", f.planes);
}

auto format_as(const Sphere& f)
{
	return fmt::format("Sphere(center:{}, radius:{})", f.c, f.r);
}

void init_math_module(py::module& m)
{
	/*py::class_<Vector2>(m, "Vector2")
		.def(py::init<>())
		.def(py::init<f32, f32>())
		.def_readwrite("x", &Vector2::x)
		.def_readwrite("y", &Vector2::y)
		.def(py::self + py::self)
		.def(py::self - py::self)
		.def(py::self * f32())
		.def(f32() * py::self )
		.def(py::self += py::self)
		.def(py::self -= py::self)
		.def(py::self *= f32())
		.def(py::self == py::self)
		.def("__repr__", [](const Vector2& vec) {return fmt::format("{}", vec); })
		;
	m.def("dot", py::overload_cast<const Vector2&, const Vector2&>(dot));
	m.def("length_squared", py::overload_cast<const Vector2&>(length_squared));
	m.def("length", py::overload_cast<const Vector2&>(length));
	m.def("normalize", py::overload_cast<Vector2&>(normalize));
	m.def("set_length", py::overload_cast<Vector2&, f32>(set_length));
	m.def("distance_squared", py::overload_cast<const Vector2&, const Vector2&>(distance_squared));
	m.def("distance", py::overload_cast<const Vector2&, const Vector2&>(distance));
	m.def("angle", py::overload_cast<const Vector2&, const Vector2&>(angle));
	m.def("max", py::overload_cast<const Vector2&, const Vector2&>(max));
	m.def("min", py::overload_cast<const Vector2&, const Vector2&>(min));
	m.def("lerp", py::overload_cast<const Vector2&, const Vector2&, f32>(lerp));
	m.attr("vec2_zero") = VECTOR2_ZERO;
	m.attr("vec2_one") = VECTOR2_ONE;
	m.attr("vec2_x") = VECTOR2_XAXIS;
	m.attr("vec2_y") = VECTOR2_YAXIS;*/


	py::class_<Vector3>(m, "Vector3")
		.def(py::init<>())
		.def(py::init<f32, f32, f32>())
		.def_readwrite("x", &Vector3::x)
		.def_readwrite("y", &Vector3::y)
		.def_readwrite("z", &Vector3::z)
		.def(py::self + py::self)
		.def(py::self - py::self)
		.def(py::self * f32())
		.def(f32() * py::self)
		.def(py::self += py::self)
		.def(py::self -= py::self)
		.def(py::self *= f32())
		.def(py::self == py::self)
		.def("__repr__", [](const Vector3& vec) {return fmt::format("{}", vec); })
		;
	m.def("dot", py::overload_cast<const Vector3&, const Vector3&>(dot));
	m.def("length_squared", py::overload_cast<const Vector3&>(length_squared));
	m.def("cross", cross);
	m.def("length", py::overload_cast<const Vector3&>(length));
	m.def("normalize", py::overload_cast<Vector3&>(normalize));
	m.def("set_length", py::overload_cast<Vector3&, f32>(set_length));
	m.def("distance_squared", py::overload_cast<const Vector3&, const Vector3&>(distance_squared));
	m.def("distance", py::overload_cast<const Vector3&, const Vector3&>(distance));
	m.def("angle", py::overload_cast<const Vector3&, const Vector3&>(angle));
	m.def("lerp", py::overload_cast<const Vector3&, const Vector3&, f32>(lerp));
	m.def("max", py::overload_cast<const Vector3&, const Vector3&>(max<Vector3>));
	m.def("min", py::overload_cast<const Vector3&, const Vector3&>(min<Vector3>));
	m.attr("vec3_zero") = VECTOR3_ZERO;
	m.attr("vec3_one") = VECTOR3_ONE;
	m.attr("vec3_x") = VECTOR3_XAXIS;
	m.attr("vec3_y") = VECTOR3_YAXIS;
	m.attr("vec3_z") = VECTOR3_ZAXIS;
	m.attr("vec3_right") = VECTOR3_RIGHT;
	m.attr("vec3_left") = VECTOR3_LEFT;
	m.attr("vec3_up") = VECTOR3_UP;
	m.attr("vec3_down") = VECTOR3_DOWN;
	m.attr("vec3_forward") = VECTOR3_FORWARD;
	m.attr("vec3_backword") = VECTOR3_BACKWARD;


	py::class_<Vector4>(m, "Vector4")
		.def(py::init<>())
		.def(py::init<f32, f32, f32, f32>())
		.def_readwrite("x", &Vector4::x)
		.def_readwrite("y", &Vector4::y)
		.def_readwrite("z", &Vector4::z)
		.def_readwrite("w", &Vector4::w)
		.def_readwrite("r", &Vector4::x)
		.def_readwrite("g", &Vector4::y)
		.def_readwrite("b", &Vector4::z)
		.def_readwrite("a", &Vector4::w)
		.def_property_readonly_static("s_test", [](py::object) { return 12; })
		.def(py::self + py::self)
		.def(py::self - py::self)
		.def(py::self * f32())
		.def(f32() * py::self)
		.def(py::self += py::self)
		.def(py::self -= py::self)
		.def(py::self *= f32())
		.def(py::self == py::self)
		.def("__repr__", [](const Vector4& vec) {return fmt::format("{}", vec); })
		;
	m.def("dot", py::overload_cast<const Vector4&, const Vector4&>(dot));
	m.def("length_squared", py::overload_cast<const Vector4&>(length_squared));
	m.def("length", py::overload_cast<const Vector4&>(length));
	m.def("normalize", py::overload_cast<Vector4&>(normalize));
	m.def("set_length", py::overload_cast<Vector4&, f32>(set_length));
	m.def("distance_squared", py::overload_cast<const Vector4&, const Vector4&>(distance_squared));
	m.def("distance", py::overload_cast<const Vector4&, const Vector4&>(distance));
	m.def("lerp", py::overload_cast<const Vector4&, const Vector4&, f32>(lerp));
	m.def("max", py::overload_cast<const Vector4&, const Vector4&>(max));
	m.def("min", py::overload_cast<const Vector4&, const Vector4&>(min));
	m.attr("vec4_zero") = VECTOR4_ZERO;
	m.attr("vec4_one") = VECTOR4_ONE;
	m.attr("vec4_x") = VECTOR4_XAXIS;
	m.attr("vec4_y") = VECTOR4_YAXIS;
	m.attr("vec4_z") = VECTOR4_ZAXIS;
	m.attr("vec4_w") = VECTOR4_WAXIS;
	m.attr("black") = COLOR4_BLACK;
	m.attr("white") = COLOR4_WHITE;
	m.attr("red") = COLOR4_RED;
	m.attr("green") = COLOR4_GREEN;
	m.attr("blue") = COLOR4_BLUE;
	m.attr("yelloe") = COLOR4_YELLOW;
	m.attr("orange") = COLOR4_ORANGE;


	py::class_<Quaternion>(m, "Quaternion")
		.def(py::init<>())
		.def(py::init<f32, f32, f32, f32>())
		.def_readwrite("x", &Quaternion::x)
		.def_readwrite("y", &Quaternion::y)
		.def_readwrite("z", &Quaternion::z)
		.def_readwrite("w", &Quaternion::w)
		.def(py::self * f32())
		.def(py::self * py::self)
		.def(py::self *= py::self)
		.def(- py::self)
		.def("__repr__", [](const Quaternion& v) {return fmt::format("{}", v); })
		;
	m.def("dot", py::overload_cast<const Quaternion&, const Quaternion&>(dot));
	m.def("normalize", py::overload_cast<Quaternion&>(normalize));
	m.def("lerp", py::overload_cast<const Quaternion&, const Quaternion&, f32>(lerp));
	m.attr("quat_identity") = QUATERNION_IDENTITY;

	//py::class_<Matrix3x3>(m, "Matrix3x3")
	//	.def(py::init<>())
	//	.def(py::init<Vector3, Vector3, Vector3>())
	//	.def_readwrite("x", &Matrix3x3::x)
	//	.def_readwrite("y", &Matrix3x3::y)
	//	.def_readwrite("z", &Matrix3x3::z)
	//	.def("__repr__", [](const Matrix3x3& v) {return fmt::format("{}", v); })
	//	;


	py::class_<Matrix4x4>(m, "Matrix4x4")
		.def(py::init<>())
		.def(py::init<Vector4, Vector4, Vector4, Vector4>())
		.def_readwrite("x", &Matrix4x4::x)
		.def_readwrite("y", &Matrix4x4::y)
		.def_readwrite("z", &Matrix4x4::z)
		.def_readwrite("t", &Matrix4x4::t)
		.def(py::self += py::self)
		.def(py::self -= py::self)
		.def(py::self *= py::self)
		.def(py::self *= f32())
		.def(py::self + py::self)
		.def(py::self - py::self)
		.def(py::self * f32())
		.def(f32() * py::self)
		.def(Vector3() * py::self)
		.def(Vector4() * py::self)
		.def(py::self * py::self)
		.def(py::self == py::self)
		.def("__repr__", [](const Matrix4x4& v) {return fmt::format("{}", v); })
		.def_static("from_axes",
			[](const Vector3& x, const Vector3& y, const Vector3& z, const Vector3& t)
			{return from_axes(x, y, z, t); }
		)
		.def_static("from_translation",
			[](const Vector3& t)
			{return from_translation(t); }
		)
		.def_static("from_quaternion",
			[](const Quaternion& r)
			{return from_quaternion_translation(r, VECTOR3_ZERO); }
		)
		.def_static("from_quaternion_translation",
			[](const Quaternion& r, const Vector3& t)
			{return from_quaternion_translation(r, t); }
		)
		;
	m.attr("mat4x4_identity") = MATRIX4X4_IDENTITY;


	py::class_<AABB>(m, "AABB")
		.def(py::init<>())
		.def(py::init<Vector3, Vector3>())
		.def_readwrite("min", &AABB::min)
		.def_readwrite("max", &AABB::max)
		.def("__repr__", [](const AABB& v) {return fmt::format("{}", v); })
		;

	py::class_<OBB>(m, "OBB")
		.def(py::init<>())
		.def(py::init<Matrix4x4, Vector3>())
		.def_readwrite("tm", &OBB::tm)
		.def_readwrite("half_extents", &OBB::half_extents)
		.def("__repr__", [](const OBB& v) {return fmt::format("{}", v); })
		;

	py::class_<Plane3>(m, "Plane3")
		.def(py::init<>())
		.def(py::init<Vector3, f32>())
		.def_readwrite("n", &Plane3::n)
		.def_readwrite("d", &Plane3::d)
		.def("__repr__", [](const Plane3& v) {return fmt::format("{}", v); })
		;

	py::class_<Frustum>(m, "Frustum")
		.def(py::init<>())
		.def(py::init<>())
		.def_readwrite("planes", &Frustum::planes)
		.def("__repr__", [](const Frustum& v) {return fmt::format("{}", v); })
		;

	py::class_<Sphere>(m, "Sphere")
		.def(py::init<>())
		.def(py::init<Vector3, f32>())
		.def_readwrite("c", &Sphere::c)
		.def_readwrite("r", &Sphere::r)
		.def("__repr__", [](const Sphere& v) {return fmt::format("{}", v); })
		;



	m.attr("plane3_zero") = PLANE3_ZERO;
	m.attr("plane3_x") = PLANE3_XAXIS;
	m.attr("plane3_y") = PLANE3_YAXIS;
	m.attr("plane3_z") = PLANE3_ZAXIS;

	m.def("ray_plane_intersection", ray_plane_intersection);
	m.def("ray_disc_intersection", ray_disc_intersection);
	m.def("ray_sphere_intersection", ray_sphere_intersection);
	m.def("ray_obb_intersection", ray_obb_intersection);
	m.def("ray_triangle_intersection", ray_triangle_intersection);
	m.def("obb_intersects_frustum", obb_intersects_frustum);
	m.def("obb_vertices", obb::to_vertices);

	m.def("from_quaternion_translation", from_quaternion_translation);
	m.def("from_quaternion", [](const Quaternion& r) {return from_quaternion_translation(r, VECTOR3_ZERO); });
	m.def("from_translation", from_translation);
	m.def("from_axes", py::overload_cast<const Vector3&, const Vector3&, const Vector3&>(from_axes));
	m.def("from_axes", py::overload_cast<const Vector3&, const Vector3&, const Vector3&, const Vector3&>(from_axes));
	m.def("transpose", py::overload_cast<Matrix3x3&>(transpose));
	m.def("transpose", py::overload_cast<Matrix4x4&>(transpose));
	m.def("invert", py::overload_cast<Matrix3x3&>(invert));
	m.def("invert", py::overload_cast<Matrix4x4&>(invert));
	//m.def()
}

PYBIND11_MODULE(crown, m) {
	m.doc() = "pybind11 example plugin"; // optional module docstring
	//m.def("add", &add, "A function that adds two numbers");
	auto m_math = m.def_submodule("math", "core.math module");
	init_math_module(m_math);
}

void ii()
{
	PyImport_AppendInittab("crown", PyInit_crown);
}
