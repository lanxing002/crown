#include "pybind11/pybind11.h"
#include "device/device.h"


namespace py = pybind11;
using namespace crown;

#include "core/math/intersection.h"
#include "core/math/types.h"
#include "core/math/vector2.inl"
#include "core/math/vector3.inl"
#include "core/math/vector4.inl"

void init_math_module(py::module& m)
{
	py::class_<Vector2>(m, "Vector2")
		.def(py::init<>())
		.def(py::init<f32, f32>())
		.def_readwrite("x", &Vector2::x)
		.def_readwrite("y", &Vector2::y);

	py::class_<Vector3>(m, "Vector3")
		.def(py::init<>())
		.def(py::init<f32, f32, f32>())
		.def_readwrite("x", &Vector3::x)
		.def_readwrite("y", &Vector3::y)
		.def_readwrite("z", &Vector3::z);


	py::class_<Vector4>(m, "Vector4")
		.def(py::init<>())
		.def(py::init<f32, f32, f32, f32>())
		.def_readwrite("x", &Vector4::x)
		.def_readwrite("y", &Vector4::y)
		.def_readwrite("z", &Vector4::z)
		.def_readwrite("w", &Vector4::w);


	//py::class_<Color4>(m, "Color4")
	//	.def(py::init<f32, f32, f32, f32>())
	//	.def_readwrite("r", &Color4::x)
	//	.def_readwrite("g", &Color4::y)
	//	.def_readwrite("b", &Color4::z)
	//	.def_readwrite("a", &Color4::w);

	py::class_<Quaternion>(m, "Quaternion")
		.def(py::init<>())
		.def(py::init<f32, f32, f32, f32>())
		.def_readwrite("x", &Quaternion::x)
		.def_readwrite("y", &Quaternion::y)
		.def_readwrite("z", &Quaternion::z)
		.def_readwrite("w", &Quaternion::w);

	py::class_<Matrix3x3>(m, "Matrix3x3")
		.def(py::init<>())
		.def(py::init<Vector3, Vector3, Vector3>())
		.def_readwrite("x", &Matrix3x3::x)
		.def_readwrite("y", &Matrix3x3::y)
		.def_readwrite("z", &Matrix3x3::z);

	py::class_<Matrix4x4>(m, "Matrix4x4")
		.def(py::init<>())
		.def(py::init<Vector4, Vector4, Vector4, Vector4>())
		.def_readwrite("x", &Matrix4x4::x)
		.def_readwrite("y", &Matrix4x4::y)
		.def_readwrite("z", &Matrix4x4::z)
		.def_readwrite("t", &Matrix4x4::t);

	py::class_<AABB>(m, "AABB")
		.def(py::init<>())
		.def(py::init<f32, Vector3>())
		.def_readwrite("min", &AABB::min)
		.def_readwrite("max", &AABB::max);

	py::class_<OBB>(m, "OBB")
		.def(py::init<>())
		.def(py::init<Matrix4x4, Vector3>())
		.def_readwrite("tm", &OBB::tm)
		.def_readwrite("half_extents", &OBB::half_extents);

	py::class_<Plane3>(m, "Plane3")
		.def(py::init<>())
		.def(py::init<Vector3, f32>())
		.def_readwrite("n", &Plane3::n)
		.def_readwrite("d", &Plane3::d);

	py::class_<Frustum>(m, "Frustum")
		.def(py::init<>())
		.def(py::init<>())
		.def_readwrite("planes", &Frustum::planes);

	py::class_<Sphere>(m, "Sphere")
		.def(py::init<>())
		.def(py::init<Vector3, f32>())
		.def_readwrite("c", &Sphere::c)
		.def_readwrite("r", &Sphere::r);

	//m.def("")

	//m.def("ray_plane_intersection", ray_plane_intersection)
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
