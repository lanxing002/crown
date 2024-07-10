#include "pybind11/pybind11.h"
#include "pybind11/operators.h"
#include "pybind11/embed.h"

#include "fmt/format.h"
#include "fmt/ranges.h"

#include "core/guid.h"
//#include "core/guid.inl"
#include "core/math/intersection.h"
#include "core/math/color4.inl"
#include "core/math/constants.h"
#include "core/math/math.h"
#include "core/math/matrix4x4.inl"
#include "core/math/obb.inl"
#include "core/math/plane3.inl"
#include "core/math/quaternion.inl"
#include "core/math/types.h"
#include "core/math/vector2.inl"
#include "core/math/vector3.inl"

#include "world/animation_state_machine.h"
#include "world/debug_line.h"
#include "world/gui.h"
#include "world/level.h"
#include "world/material.h"
#include "world/physics_world.h"
#include "world/render_world.h"
#include "world/scene_graph.h"
#include "world/sound_world.h"
#include "world/types.h"
#include "world/unit_manager.h"
#include "world/world.h"

#include "resource/level_resource.h"
#include "resource/resource_id.inl"
#include "resource/resource_manager.h"
#include "resource/resource_package.h"
#include "resource/sound_resource.h"

#include "core/containers/array.inl"
#include "core/containers/hash_map.inl"
#include "core/containers/hash_set.inl"
#include "core/containers/vector.inl"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_id.inl"
#include "core/strings/string_stream.inl"
#include "device/console_server.h"
#include "device/device.h"
#include "device/input_device.h"
#include "device/input_manager.h"
#include "device/profiler.h"

namespace py = pybind11;
using namespace crown;

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

auto format_as(const Matrix4x4& m)
{
	return fmt::format("Matrix4x4(x:{}, y:{}, z:{}, t:{})", m.x, m.y, m.z, m.t);
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

namespace PYBIND11_NAMESPACE {
	namespace detail {
		template <> struct type_caster<StringId64> {
		public:
			/**
			 * This macro establishes the name 'StringId64' in
			 * function signatures and declares a local variable
			 * 'value' of type StringId64
			 */
			PYBIND11_TYPE_CASTER(StringId64, const_name("StringId64"));

			/**
			 * Conversion part 1 (Python->C++): convert a PyObject into a StringId64
			 * instance or return false upon failure. The second argument
			 * indicates whether implicit conversions should be applied.
			 */
			bool load(handle src, bool) {
				bool ret = false;

				if (py::isinstance<py::str>(src))
				{
					auto str = src.cast<std::string>();
					value.hash(str.c_str(), str.size());
					ret = true;
				}

				return ret && !PyErr_Occurred();
			}

			/**
			 * Conversion part 2 (C++ -> Python): convert an StringId64 instance into
			 * a Python object. The second and third arguments are used to
			 * indicate the return value policy and parent object (for
			 * ``return_value_policy::reference_internal``) and are generally
			 * ignored by implicit casters.
			 */
			static handle cast(StringId64 src, return_value_policy /* policy */, handle /* parent */) {
				char buf[STRING_ID64_BUF_LEN + 1];
				src.to_string(buf, STRING_ID64_BUF_LEN + 1);
				return py::str(buf);
			}
		};

		template<>
		struct type_caster<StringId32>
		{
			PYBIND11_TYPE_CASTER(StringId32, const_name("StringId32"));

			bool load(handle src, bool)
			{
				bool ret = false;
				if (py::isinstance<py::str>(src))
				{
					auto str = src.cast<std::string>();
					value.hash(str.c_str(), str.size());
					ret = true;
				}

				return ret && !PyErr_Occurred();
			}

			static handle cast(StringId32 src, return_value_policy /* policy */, handle /* parent */)
			{
				char buf[STRING_ID32_BUF_LEN + 1];
				src.to_string(buf, STRING_ID32_BUF_LEN + 1);
				return py::str(buf);
			}
		};
	}
} // namespace PYBIND11_NAMESPACE::detail

void init_input(py::module& m)
{
#define KEYBOARD(name)																	\
	[]()																				\
	{																					\
		return device()->_input_manager->keyboard()->##name();							\
	}																					

#define KEYBOARDP(name, tparam)															\
	[](tparam p)																		\
	{																					\
		return device()->_input_manager->keyboard()->##name(p);							\
	}		

	auto key_m = m.def_submodule("keyboard", "crown.core.input.keyboard");
	key_m.def("name", KEYBOARD(name));
	key_m.def("connected", KEYBOARD(connected));
	key_m.def("num_buttons", KEYBOARD(num_buttons));
	key_m.def("num_axes", KEYBOARD(num_axes));
	key_m.def("pressed", KEYBOARDP(pressed, u8));
	key_m.def("released", KEYBOARDP(released, u8));
	key_m.def("any_released", KEYBOARD(any_released));
	key_m.def("any_pressed", KEYBOARD(any_pressed));
	key_m.def("button", KEYBOARDP(button, u8));
	key_m.def("button_name", KEYBOARDP(button_name, u8));
	key_m.def("button_id", KEYBOARDP(button_id, StringId32));

#undef KEYBOARD
#undef KEYBOARDP

#define MOUSE(name)																\
	[]()																		\
	{																			\
		return device()->_input_manager->mouse()->##name();						\
	}

#define MOUSEP(name, tparam)													\
	[](tparam p)																\
	{																			\
		return device()->_input_manager->mouse()->##name(p);					\
	}

	auto mouse_m = m.def_submodule("mouse", "mouse.core.input.mouse");
	mouse_m.def("name", MOUSE(name));
	mouse_m.def("connected", MOUSE(connected));
	mouse_m.def("num_buttons", MOUSE(num_buttons));
	mouse_m.def("num_axes", MOUSE(num_axes));
	mouse_m.def("pressed", MOUSEP(pressed, u8));
	mouse_m.def("released", MOUSEP(released, u8));
	mouse_m.def("any_pressed", MOUSE(any_pressed));
	mouse_m.def("any_released", MOUSE(any_released));
	mouse_m.def("button", MOUSEP(button, u8));
	mouse_m.def("axis", MOUSEP(axis, u8));
	mouse_m.def("button_name", MOUSEP(button_name, u8));
	mouse_m.def("axis_name",   MOUSEP(axis_name, u8));
	mouse_m.def("button_id",   MOUSEP(button_id, StringId32));
	mouse_m.def("axis_id", MOUSEP(axis_id, StringId32));

#undef MOUSE
#undef MOUSEP

#define TOUCH(name)                                                 \
	[]()															\
	{                                                               \
		return device()->_input_manager->touch()->##name();			\
	}

#define TOUCHP(name, tparam)                                         \
	[](tparam p)													\
	{                                                               \
		return device()->_input_manager->touch()->##name(p);		\
	}

	auto touch_m = m.def_submodule("touch", "touch.core.input.mouse");
	touch_m.def("name", TOUCH(name));
	touch_m.def("connected", TOUCH(connected));
	touch_m.def("num_buttons", TOUCH(num_buttons));
	touch_m.def("num_axes", TOUCH(num_axes));
	touch_m.def("pressed", TOUCHP(pressed, u8));
	touch_m.def("released", TOUCHP(released, u8));
	touch_m.def("any_pressed", TOUCH(any_pressed));
	touch_m.def("any_released", TOUCH(any_released));
	touch_m.def("button", TOUCHP(button, u8));
	touch_m.def("axis", TOUCHP(axis, u8));
	touch_m.def("button_name", TOUCHP(button_name, u8));
	touch_m.def("axis_name", TOUCHP(axis_name, u8));
	touch_m.def("button_id", TOUCHP(button_id, StringId32));
	touch_m.def("axis_id", TOUCHP(axis_id, StringId32));

#undef TOUCH
#undef TOUCHP

#define PAD(index, name)											\
	[]()															\
	{                                                               \
		return device()->_input_manager->joypad(index)->##name();	\
	}

#define PADP(index, name, tparam)											\
	[](tparam p)															\
	{                                                               \
		return device()->_input_manager->joypad(index)->##name(p);	\
	}

	auto pad1_m = m.def_submodule("pad1", "crown.core.input.pad1");
	pad1_m.def("name", PAD(0, name));
	pad1_m.def("connected", PAD(0, connected));
	pad1_m.def("num_buttons", PAD(0, num_buttons));
	pad1_m.def("num_axes", PAD(0, num_axes));
	pad1_m.def("pressed", PADP(0, pressed, u8));
	pad1_m.def("released", PADP(0, released, u8));
	pad1_m.def("any_pressed", PAD(0, any_pressed));
	pad1_m.def("any_released", PAD(0, any_released));
	pad1_m.def("button", PADP(0, button, u8));
	pad1_m.def("axis", PADP(0, axis, u8));
	pad1_m.def("button_name", PADP(0, button_name, u8));
	pad1_m.def("axis_name", PADP(0, button_id, StringId32));
	pad1_m.def("axis_id", PADP(0, axis_id, StringId32));
	pad1_m.def("deadzone", [](u8 p)
		{
			DeadzoneMode::Enum deadzone_mode;
			auto val = device()->_input_manager->joypad(0)->deadzone(p, &deadzone_mode);
			return std::make_tuple(val, deadzone_mode);
		});
	pad1_m.def("set_deadzone", [](u8 id, DeadzoneMode::Enum deadzone_mode, f32 deadzone_size)
		{ return device()->_input_manager->joypad(0)->set_deadzone(id, deadzone_mode, deadzone_size); });

#undef PAD
#undef PADP
}

void init_base(py::module& m)
{
	py::class_<Vector2>(m, "Vector2")
		.def_readwrite("x", &Vector2::x)
		.def_readwrite("y", &Vector2::y)
		;

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

	py::class_<Quaternion>(m, "Quaternion")
		.def(py::init<>())
		.def(py::init<f32, f32, f32, f32>())
		.def(py::init(&from_axis_angle))
		.def_readwrite("x", &Quaternion::x)
		.def_readwrite("y", &Quaternion::y)
		.def_readwrite("z", &Quaternion::z)
		.def_readwrite("w", &Quaternion::w)
		.def(py::self * f32())
		.def(py::self * py::self)
		.def(py::self *= py::self)
		.def(-py::self)
		.def("__repr__", [](const Quaternion& v) {return fmt::format("{}", v); })
		;

	py::class_<Matrix4x4>(m, "Matrix4x4")
		.def(py::init<>())
		.def(py::init<Vector4, Vector4, Vector4, Vector4>())
		.def_readwrite("x", &Matrix4x4::x)
		.def_readwrite("y", &Matrix4x4::y)
		.def_readwrite("z", &Matrix4x4::z)
		.def_readwrite("t", &Matrix4x4::t)
		.def_property("translation",
			[](const Matrix4x4& mat) {return translation(mat); },
			[](Matrix4x4& self, Vector3 vec) {set_translation(self, vec); }
		)
		.def_property_readonly("rotation", [](const Matrix4x4& mat) {return rotation(mat); })
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


	py::class_<StringId32>(m, "StringId32")
		.def(py::init<>())
		.def(py::init<u32>())
		.def(py::init<const char*>())
		.def(py::init<const char*, u32>())
		.def("hash", &StringId32::hash)
		.def("parse", &StringId32::parse)
		.def("__repr__", [](const StringId32& str)
			{
				std::vector<char> buf(STRING_ID32_BUF_LEN + 1, ' ');
				return std::string(str.to_string(buf.data(), STRING_ID32_BUF_LEN + 1));
			})
		;

	py::class_<StringId64>(m, "StringId64")
		.def(py::init<>())
		.def(py::init<u32>())
		.def(py::init<const char*>())
		.def(py::init<const char*, u64>())
		.def("hash", &StringId64::hash)
		.def("parse", &StringId64::parse)
		.def("__repr__", [](const StringId64& str)
			{
				std::vector<char> buf(STRING_ID64_BUF_LEN + 1, ' ');
				return std::string(str.to_string(buf.data(), STRING_ID64_BUF_LEN + 1));
			})
		;

	py::class_<UnitId>(m, "UnitId")
		.def(py::init<>())
		.def(py::init<u32>())
		.def("index", &UnitId::index)
		.def("id", &UnitId::id)
		.def("is_valid", &UnitId::is_valid)
		;

	py::class_<SoundResource>(m, "SoundResource")
		.def(py::init<>())
		.def_readwrite("version", &SoundResource::version)
		.def_readwrite("size", &SoundResource::size)
		.def_readwrite("sample_rate", &SoundResource::sample_rate)
		.def_readwrite("avg_bytes_ps", &SoundResource::avg_bytes_ps)
		.def_readwrite("channels", &SoundResource::channels)
		.def_readwrite("block_size", &SoundResource::block_size)
		.def_readwrite("bits_ps", &SoundResource::bits_ps)
		.def_readwrite("sound_type", &SoundResource::sound_type)
		;

	py::enum_<InputEventType::Enum>(m, "InputEventType")
		.value("BUTTON_PRESSED", InputEventType::Enum::BUTTON_PRESSED)
		.value("BUTTON_RELEASED", InputEventType::Enum::BUTTON_RELEASED)
		.value("AXIS_CHANGED", InputEventType::Enum::AXIS_CHANGED)
		.value("COUNT", InputEventType::Enum::COUNT)
		.export_values()
		;

	py::class_<InputEvent>(m, "InputEvent")
		.def_readwrite("id", &InputEvent::id)
		.def_readwrite("type", &InputEvent::type)
		.def_readwrite("value", &InputEvent::value)
		//.def_readwrite("device", &InputEvent::device)
		;

	py::enum_<LightType::Enum>(m, "LightTypeType")
		.value("DIRECTIONAL", LightType::Enum::DIRECTIONAL)
		.value("OMNI", LightType::Enum::OMNI)
		.value("SPOT", LightType::Enum::SPOT)
		.export_values();

	py::enum_<MouseCursor::Enum>(m, "MouseCursorType")
		.value("ARROW", MouseCursor::Enum::ARROW)
		.value("TEXT_INPUT", MouseCursor::Enum::TEXT_INPUT)
		.value("CORNER_TOP_LEFT", MouseCursor::Enum::CORNER_TOP_LEFT)
		.value("CORNER_TOP_RIGHT", MouseCursor::Enum::CORNER_TOP_RIGHT)
		.value("CORNER_BOTTOM_LEFT", MouseCursor::Enum::CORNER_BOTTOM_LEFT)
		.value("CORNER_BOTTOM_RIGHT", MouseCursor::Enum::CORNER_BOTTOM_RIGHT)
		.value("SIZE_HORIZONTAL", MouseCursor::Enum::SIZE_HORIZONTAL)
		.value("SIZE_VERTICAL", MouseCursor::Enum::SIZE_VERTICAL)
		.value("WAIT", MouseCursor::Enum::WAIT)
		.export_values();

	py::enum_<CursorMode::Enum>(m, "CursorMode")
		.value("NORMAL", CursorMode::NORMAL)
		.value("DISABLED", CursorMode::DISABLED)
		.export_values();

	py::enum_<ProjectionType::Enum>(m, "ProjectionType")
		.value("PERSPECTIVE", ProjectionType::PERSPECTIVE)
		.value("ORTHOGRAPHIC", ProjectionType::ORTHOGRAPHIC)
		.export_values();

	py::enum_<DeadzoneMode::Enum>(m, "DeadzoneMode")
		.value("RAW", DeadzoneMode::RAW)
		.value("INDEPENDENT", DeadzoneMode::INDEPENDENT)
		.value("CIRCULAR", DeadzoneMode::CIRCULAR)
		.export_values();
}

void init_math_module(py::module& m)
{
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


	m.def("dot", py::overload_cast<const Quaternion&, const Quaternion&>(dot));
	m.def("normalize", py::overload_cast<Quaternion&>(normalize));
	m.def("lerp", py::overload_cast<const Quaternion&, const Quaternion&, f32>(lerp));
	m.attr("quat_identity") = QUATERNION_IDENTITY;

	m.attr("mat4x4_identity") = MATRIX4X4_IDENTITY;

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
	m.def("from_axes", py::overload_cast<const Vector3&, const Vector3&, const Vector3&, const Vector3&>(from_axes));
	m.def("transpose", py::overload_cast<Matrix4x4&>(transpose));
	m.def("invert", py::overload_cast<Matrix4x4&>(invert));
	m.def("from_axis_angle", from_axis_angle);
}


#include <iostream>
struct  ProDemo
{
	Vector3 vec3;
	int ii = 0;

	~ProDemo()
	{
		std::cout << "deconstruct prodemo" << std::endl;
		vec3.x = 100;
		vec3.y = 200;
		vec3.z = 100;
		ii = 200;
	}

	Vector3 v;

	void set(Vector3 vv) { v = vv; }
	Vector3 get() { return v; }
};

#ifdef DemoTest
ProDemo g_;
ProDemo* pg_ = new ProDemo();
#endif // DemoTest

void init_world(py::module& m)
{
	auto world_ = py::class_<World>(m, "World");
	auto level_ = py::class_<Level>(m, "Level");

#define BIND_INSTANCE_ID(inst_name) \
	py::class_<inst_name>(m, #inst_name)	\
		.def(py::init<>())			\
		.def(py::init<u32>())		\
		.def("is_valid", [](const inst_name& inst){return inst.i != UINT32_MAX;})		\
		;
	;

	BIND_INSTANCE_ID(TransformInstance)
	BIND_INSTANCE_ID(CameraInstance)
	BIND_INSTANCE_ID(MeshInstance)
	BIND_INSTANCE_ID(SpriteInstance)
	BIND_INSTANCE_ID(LightInstance)
	BIND_INSTANCE_ID(ColliderInstance)
	BIND_INSTANCE_ID(ActorInstance)
	BIND_INSTANCE_ID(JointInstance)
	BIND_INSTANCE_ID(ScriptInstance)
	BIND_INSTANCE_ID(StateMachineInstance)

#undef BIND_INSTANCE_ID

#ifdef DemoTest
	py::class_<ProDemo>(m, "ProDemo")
		.def(py::init<>())
		.def_readwrite("vec3", &ProDemo::vec3)
		.def_readwrite("ii", &ProDemo::ii)
		.def_property("pvec3", &ProDemo::get, &ProDemo::set)
		.def_property_readonly_static("g_", [](const py::object&) {return g_; })
		.def_property_readonly_static("pg_", [](const py::object&) {return pg_; })
		.def_static("get_pg_ref", []() {return pg_; }, py::return_value_policy::reference)
	//	.def_static("get_pg",[]() {return pg_; })
		;
#endif	

	// level 中间结构体
	py::class_<LevelResource>(m, "LevelResource")
		.def(py::init<>([]()
			{
				assert(false && "should not construct LevelResource in py");
				return LevelResource();
			}))
		;

		py::class_<ResourcePackage>(m, "ResourcePackage")
		.def("load", &ResourcePackage::load)
		.def("unload", &ResourcePackage::unload)
		.def("flush", &ResourcePackage::flush)
		.def("has_loaded", &ResourcePackage::has_loaded)
		;

	py::class_<Material>(m, "Material")
		.def("set_float", &Material::set_float)
		.def("set_vector2", &Material::set_vector2)
		.def("set_vector3", &Material::set_vector3)
		.def("set_vector4", &Material::set_vector4)
		.def("set_matrix4x4", &Material::set_matrix4x4)
		;

	py::class_<Gui>(m, "Gui")
		.def("move", &Gui::move)
		.def("triangle", &Gui::triangle)
		.def("rect", &Gui::rect)
		.def("image", &Gui::image)
		.def("image_uv", &Gui::image_uv)
		.def("text", &Gui::text)
		.def("text", &Gui::text)
		;

	py::class_<UnitManager>(m, "UnitManager")
		.def("create", py::overload_cast<World&>(&UnitManager::create))
		.def("create", py::overload_cast<>(&UnitManager::create))
		.def("alive", &UnitManager::alive)
		;

	py::class_<RaycastHit>(m, "RaycastHit")
		.def(py::init<>())
		.def_readwrite("position", &RaycastHit::position)
		.def_readwrite("normal", &RaycastHit::normal)
		.def_readwrite("time", &RaycastHit::time)
		.def_readwrite("unit", &RaycastHit::unit)
		.def_readwrite("actor", &RaycastHit::actor)
		;


	py::class_<CameraDesc>(m, "CameraDesc")
		.def(py::init<>())
		.def_readwrite("type", &CameraDesc::type)
		.def_readwrite("fov", &CameraDesc::fov)
		.def_readwrite("near_range", &CameraDesc::near_range)
		.def_readwrite("far_range", &CameraDesc::far_range)
		;

	py::class_<JointDesc>(m, "JointDesc")
		.def(py::init<>())
		.def_readwrite("type", &JointDesc::type)
		.def_readwrite("anchor_0", &JointDesc::anchor_0)
		.def_readwrite("anchor_1", &JointDesc::anchor_1)
		.def_readwrite("breakable", &JointDesc::breakable)
		;

	py::class_<LightDesc>(m, "LightDesc")
		.def(py::init<>())
		.def_readwrite("type", &LightDesc::type)
		.def_readwrite("range", &LightDesc::range)
		.def_readwrite("intensity", &LightDesc::intensity)
		.def_readwrite("spot_angle", &LightDesc::spot_angle)
		.def_readwrite("color", &LightDesc::color)
		;

	py::class_<SpriteRendererDesc>(m, "SpriteRendererDesc")
		.def(py::init<>())
		.def_readwrite("sprite_resource", &SpriteRendererDesc::sprite_resource)
		.def_readwrite("material_resource", &SpriteRendererDesc::material_resource)
		.def_readwrite("layer", &SpriteRendererDesc::layer)
		.def_readwrite("depth", &SpriteRendererDesc::depth)
		.def_readwrite("visible", &SpriteRendererDesc::visible)
		;

	py::class_<MeshRendererDesc>(m, "MeshRendererDesc")
		.def(py::init<>())
		.def_readwrite("mesh_resource", &MeshRendererDesc::mesh_resource)
		.def_readwrite("material_resource", &MeshRendererDesc::material_resource)
		.def_readwrite("geometry_name", &MeshRendererDesc::geometry_name)
		.def_readwrite("visible", &MeshRendererDesc::visible)
		;

	py::class_<PhysicsWorld>(m, "PhysicsWorld")
		.def("actor", &PhysicsWorld::actor)
		.def("actor_destroy", &PhysicsWorld::actor_destroy)
		.def("actor_world_position", &PhysicsWorld::actor_world_position)
		.def("actor_world_rotation", &PhysicsWorld::actor_world_rotation)
		.def("actor_world_pose", &PhysicsWorld::actor_world_pose)
		.def("actor_teleport_world_position", &PhysicsWorld::actor_teleport_world_position)
		.def("actor_teleport_world_rotation", &PhysicsWorld::actor_teleport_world_rotation)
		.def("actor_teleport_world_pose", &PhysicsWorld::actor_teleport_world_pose)
		.def("actor_center_of_mass", &PhysicsWorld::actor_center_of_mass)
		.def("actor_enable_gravity", &PhysicsWorld::actor_enable_gravity)
		.def("actor_disable_gravity", &PhysicsWorld::actor_disable_gravity)
		.def("actor_enable_collision", &PhysicsWorld::actor_enable_collision)
		.def("actor_disable_collision", &PhysicsWorld::actor_disable_collision)
		.def("actor_set_collision_filter", &PhysicsWorld::actor_set_collision_filter)
		.def("actor_set_kinematic", &PhysicsWorld::actor_set_kinematic)
		.def("actor_is_static", &PhysicsWorld::actor_is_static)
		.def("actor_is_dynamic", &PhysicsWorld::actor_is_dynamic)
		.def("actor_is_kinematic", &PhysicsWorld::actor_is_kinematic)
		.def("actor_is_nonkinematic", &PhysicsWorld::actor_is_nonkinematic)
		.def("actor_linear_damping", &PhysicsWorld::actor_linear_damping)
		.def("actor_set_linear_damping", &PhysicsWorld::actor_set_linear_damping)
		.def("actor_angular_damping", &PhysicsWorld::actor_angular_damping)
		.def("actor_linear_velocity", &PhysicsWorld::actor_linear_velocity)
		.def("actor_set_linear_velocity", &PhysicsWorld::actor_set_linear_velocity)
		.def("actor_angular_velocity", &PhysicsWorld::actor_angular_velocity)
		.def("actor_set_angular_velocity", &PhysicsWorld::actor_set_angular_velocity)
		.def("actor_add_impulse", &PhysicsWorld::actor_add_impulse)
		.def("actor_add_impulse_at", &PhysicsWorld::actor_add_impulse_at)
		.def("actor_add_torque_impulse", &PhysicsWorld::actor_add_torque_impulse)
		.def("actor_push", &PhysicsWorld::actor_push)
		.def("actor_push_at", &PhysicsWorld::actor_push_at)
		.def("actor_is_sleeping", &PhysicsWorld::actor_is_sleeping)
		.def("actor_wake_up", &PhysicsWorld::actor_wake_up)
		.def("joint_create", &PhysicsWorld::joint_create)
		.def("gravity", &PhysicsWorld::gravity)
		.def("set_gravity", &PhysicsWorld::set_gravity)
		//.def("cast_ray_all", [](PhysicsWorld& pw, Array<RaycastHit>& hits, const Vector3& from, const Vector3& dir, f32 le) {})
		.def("cast_sphere", &PhysicsWorld::cast_sphere)
		.def("cast_box", &PhysicsWorld::cast_box)
		.def("enable_debug_drawing", &PhysicsWorld::enable_debug_drawing)
		;

	py::class_<SoundWorld>(m, "SoundWorld")
		.def("stop_all", &SoundWorld::stop_all)
		.def("pause_all", &SoundWorld::pause_all)
		.def("resume_all", &SoundWorld::resume_all)
		.def("is_playing", &SoundWorld::is_playing)
		.def("pause_all", &SoundWorld::pause_all)
		;

	py::class_<AnimationStateMachine>(m, "AnimationStateMachine")
		.def("instance", &AnimationStateMachine::instance)
		.def("trigger", &AnimationStateMachine::trigger)
		.def("variable_id", &AnimationStateMachine::variable_id)
		.def("variable", &AnimationStateMachine::variable)
		.def("set_variable", &AnimationStateMachine::set_variable)
		;

	py::class_<SceneGraph>(m, "SceneGraph")
		.def("create", py::overload_cast<UnitId, const Matrix4x4&>(&SceneGraph::create))
		.def("create", py::overload_cast<UnitId, const Vector3&, const Quaternion&, const Vector3&>(&SceneGraph::create))
		.def("destroy", &SceneGraph::destroy)
		.def("instance", &SceneGraph::instance)
		.def("local_position", &SceneGraph::local_position)
		.def("local_rotation", &SceneGraph::local_rotation)
		.def("local_scale", &SceneGraph::local_scale)
		.def("local_pose", &SceneGraph::local_pose)
		.def("world_position", &SceneGraph::world_position)
		.def("world_rotation", &SceneGraph::world_rotation)
		.def("world_pose", &SceneGraph::world_pose)
		.def("set_local_position", &SceneGraph::set_local_position)
		.def("set_local_rotation", &SceneGraph::set_local_rotation)
		.def("set_local_scale", &SceneGraph::set_local_scale)
		.def("set_local_pose", &SceneGraph::set_local_pose)
		.def("link", &SceneGraph::link)
		.def("unlink", &SceneGraph::unlink)
		;

	py::class_<DebugLine>(m, "DebugLine")
		.def("add_line", &DebugLine::add_line)
		.def("add_axes", &DebugLine::add_axes)
		.def("add_arc", &DebugLine::add_arc)
		.def("add_circle", &DebugLine::add_circle)
		.def("add_cone", &DebugLine::add_cone)
		.def("add_sphere", &DebugLine::add_sphere)
		.def("add_obb", &DebugLine::add_obb)
		.def("add_frustum", &DebugLine::add_frustum)
		.def("reset", &DebugLine::reset)
		.def("submit", &DebugLine::submit)
		;

	py::class_<RenderWorld>(m, "RenderWorld")
		.def("mesh_create", &RenderWorld::mesh_create)
		.def("mesh_destroy", &RenderWorld::mesh_destroy)
		.def("mesh_instance", &RenderWorld::mesh_instance)
		.def("mesh_obb", &RenderWorld::mesh_obb)
		.def("mesh_cast_ray", &RenderWorld::mesh_cast_ray)
		.def("mesh_material", &RenderWorld::mesh_material)
		.def("mesh_set_material", &RenderWorld::mesh_set_material)
		.def("mesh_set_visible", &RenderWorld::mesh_set_visible)
		.def("sprite_create", &RenderWorld::sprite_create)
		.def("sprite_destroy", &RenderWorld::sprite_destroy)
		.def("sprite_instance", &RenderWorld::sprite_instance)
		.def("sprite_set_sprite", &RenderWorld::sprite_set_sprite)
		.def("sprite_material", &RenderWorld::sprite_material)
		.def("sprite_set_material", &RenderWorld::sprite_set_material)
		.def("sprite_set_frame", &RenderWorld::sprite_set_frame)
		.def("sprite_set_visible", &RenderWorld::sprite_set_visible)
		.def("sprite_flip_x", &RenderWorld::sprite_flip_x)
		.def("sprite_flip_y", &RenderWorld::sprite_flip_y)
		.def("sprite_set_layer", &RenderWorld::sprite_set_layer)
		.def("sprite_set_depth", &RenderWorld::sprite_set_depth)
		.def("sprite_obb", &RenderWorld::sprite_obb)
		.def("sprite_cast_ray", &RenderWorld::sprite_cast_ray)
		.def("light_create", &RenderWorld::light_create)
		.def("light_destroy", &RenderWorld::light_destroy)
		.def("light_instance", &RenderWorld::light_instance)
		.def("light_type", &RenderWorld::light_type)
		.def("light_color", &RenderWorld::light_color)
		.def("light_range", &RenderWorld::light_range)
		.def("light_intensity", &RenderWorld::light_intensity)
		.def("light_spot_angle", &RenderWorld::light_spot_angle)
		.def("light_set_type", &RenderWorld::light_set_type)
		.def("light_set_color", &RenderWorld::light_set_color)
		.def("light_set_range", &RenderWorld::light_set_range)
		.def("light_set_intensity", &RenderWorld::light_set_intensity)
		.def("light_set_spot_angle", &RenderWorld::light_set_spot_angle)
		.def("light_debug_draw", &RenderWorld::light_debug_draw)
		.def("enable_debug_drawing", &RenderWorld::enable_debug_drawing)
		.def("selection", [](RenderWorld& world, UnitId unit, bool b_insert)
			{ b_insert ? hash_set::insert(world._selection, unit)
			: hash_set::remove(world._selection, unit); })
		;

	world_
		//.def("spawn_unit", [](World& world, StringId64 name) { return world.spawn_unit(name); })
		.def("spawn_empty_unit", &World::spawn_empty_unit)
		.def("destroy_unit", &World::destroy_unit)
		.def("num_units", &World::num_units)
		.def("spawn_unit", &World::spawn_unit,
			py::arg("name"),
			py::arg("pos") = VECTOR3_ZERO,
			py::arg("rot") = QUATERNION_IDENTITY,
			py::arg("scl") = VECTOR3_ONE)
		.def("unit_by_name", &World::unit_by_name)
		.def("camera_create", &World::camera_create)
		.def("camera_destroy", &World::camera_destroy)
		.def("camera_instance", &World::camera_instance)
		.def("camera_set_projection_type", &World::camera_set_projection_type)
		.def("camera_projection_type", &World::camera_projection_type)
		.def("camera_fov", &World::camera_fov)
		.def("camera_set_fov", &World::camera_set_fov)
		.def("camera_near_clip_distance", &World::camera_near_clip_distance)
		.def("camera_set_near_clip_distance", &World::camera_set_near_clip_distance)
		.def("camera_far_clip_distance", &World::camera_far_clip_distance)
		.def("camera_set_orthographic_size", &World::camera_set_orthographic_size)
		.def("camera_screen_to_world", &World::camera_screen_to_world)
		.def("camera_world_to_screen", &World::camera_world_to_screen)
		.def("update_animations", &World::update_animations)
		.def("update_scene", &World::update_scene)
		.def("update", &World::update)
		.def("play_sound", py::overload_cast<const SoundResource&, const bool, const f32, const Vector3&, const f32>(&World::play_sound))
		.def("play_sound", py::overload_cast<StringId64, const bool, const f32, const Vector3&, const f32>(&World::play_sound))
		.def("stop_sound", &World::stop_sound)
		.def("link_sound", &World::link_sound)
		.def("set_listener_pose", &World::set_listener_pose)
		.def("set_sound_range", &World::set_sound_range)
		.def("set_sound_volume", &World::set_sound_volume)
		.def("create_debug_line", &World::create_debug_line)
		.def("destroy_debug_line", &World::destroy_debug_line)
		.def("create_screen_gui", &World::create_screen_gui)
		.def("destroy_gui", &World::destroy_gui)
		.def("load_level", &World::load_level, py::return_value_policy::reference)
		.def_readwrite("scene_graph", &World::_scene_graph)
		//.def_property_readonly("scene_graph",
		//	py::cpp_function([](const World& world) {return world._scene_graph; }, py::return_value_policy::reference));
		.def_readwrite("render_world", &World::_render_world)
		.def_readwrite("physics_world", &World::_physics_world)
		.def_readwrite("sound_world", &World::_sound_world)
		.def_readwrite("animation_state_machine", &World::_animation_state_machine)
		.def("disable_unit_callbacks", &World::disable_unit_callbacks)
		.def_readwrite("scene_graph", &World::_scene_graph)
		;

	level_.def(py::init<>([](UnitManager& um, World& w, const LevelResource& lr)
		{
			assert(false && "should not construct level in py");
			return Level(default_allocator(), um, w, lr);
		}));

	//m.def_property_readonly_static("g_", [](const py::object&) {return g_; });
	//m.attr("g_", []() {return device(); });

	py::class_<Device>(m, "Device")
		.def_property_readonly_static("g_device", [](const py::object&) {return device(); })
		.def("argv", [](Device& device) {
				auto argc = device.argc();
				auto argv = device.argv();
				std::vector<std::string> args;
				for (int i = 0; i < argc; i++)
					args.push_back(argv[i]);
				return args;
			})
		.def_property_readonly("platform", [](py::object) {return CROWN_PLATFORM_NAME; })
		.def_property_readonly("architecture", [](py::object) {return CROWN_ARCH_NAME; })
		.def_property_readonly("version", [](py::object) {return CROWN_VERSION; })
		.def("quit", &Device::quit)
		.def("resolution", &Device::resolution)
		.def("create_world", &Device::create_world, py::return_value_policy::reference)
		.def("destroy_world", &Device::destroy_world)
		.def("render", &Device::render)
		.def("create_resource_package", &Device::create_resource_package)
		.def("destroy_resource_package", &Device::destroy_resource_package)
		.def("create_resource_package", &Device::create_resource_package)
		.def("screenshot", &Device::screenshot)
		.def("input_events", [](Device& device)
			{
				InputEvent* events = device._input_manager->_events;
				u32 num_events = device._input_manager->_num_events;
				return 1;
			})
		;

	//py::class_<Display>(m, "Display")
	//	.def("modes", [](Display& display)
	//		{
	//			TempAllocator1024 ta;
	//			Array<DisplayMode> modes(ta);
	//			display.modes(modes);
	//			std::vector<DisplayMode> ret;
	//			for (u32 i = 0; i < array::size(modes); i++)
	//				ret.push_back(modes[i]);

	//			return ret;
	//		})
	//	.def("set_mode", Display::set_mode);

	py::class_<Window>(m, "Window")
		.def("show", &Window::show)
		.def("hide", &Window::hide)
		.def("resize", &Window::resize)
		.def("move", &Window::move)
		.def("minimize", &Window::minimize)
		.def("maximize", &Window::maximize)
		.def("restore", &Window::restore)
		.def("title", &Window::title)
		.def("set_title", &Window::set_title)
		.def("show_cursor", &Window::show_cursor)
		.def("set_fullscreen", &Window::set_fullscreen)
		.def("set_cursor", &Window::set_cursor)
		.def("set_cursor_mode", &Window::set_cursor_mode)
		;
}

PYBIND11_MODULE(crown, m) {
	m.doc() = "pybind11 example plugin"; // optional module docstring
	//m.def("add", &add, "A function that adds two numbers");
	auto m_math = m.def_submodule("math", "crown.math module");

	init_base(m);
	init_input(m);
	init_math_module(m_math);

	init_world(m);
}

void ii()
{
	PyImport_AppendInittab("crown", PyInit_crown);
}
