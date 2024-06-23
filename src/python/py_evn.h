#pragma once
#include "config.h"
#include "core/math/random.h"
#include "core/math/types.h"
#include "core/types.h"
#include "device/types.h"
#include "lua/lua_stack.h"
#include "resource/types.h"
//#include "Python.h"


namespace crown
{
#define PY_MAX_VECTOR3 (CROWN_PY_MAX_VECTOR3_SIZE / sizeof(Vector3))
	CE_STATIC_ASSERT(CROWN_PY_MAX_VECTOR3_SIZE % sizeof(Vector3) == 0);

#define PY_MAX_QUATERNION (CROWN_PY_MAX_QUATERNION_SIZE / sizeof(Quaternion))
	CE_STATIC_ASSERT(CROWN_PY_MAX_QUATERNION_SIZE % sizeof(Quaternion) == 0);

#define PY_MAX_MATRIX4X4 (CROWN_PY_MAX_MATRIX4X4_SIZE / sizeof(Matrix4x4))
	CE_STATIC_ASSERT(CROWN_PY_MAX_MATRIX4X4_SIZE % sizeof(Matrix4x4) == 0);

	//int report(bool );

	/// Wraps a subset of Lua functions and provides utilities for extending Lua.
	///
	/// @ingroup Lua
	struct PyWrapper
	{
		u32 _num_vec3;
		CE_ALIGN_DECL(4, Vector3 _vec3[PY_MAX_VECTOR3]);
		CE_STATIC_ASSERT(4 == 1 + PY_VECTOR3_MARKER_MASK);
		u32 _num_quat;
		CE_ALIGN_DECL(16, Quaternion _quat[PY_MAX_QUATERNION]);
		CE_STATIC_ASSERT(16 == 1 + PY_QUATERNION_MARKER_MASK);
		u32 _num_mat4;
		CE_ALIGN_DECL(64, Matrix4x4 _mat4[PY_MAX_MATRIX4X4]);
		CE_STATIC_ASSERT(64 == 1 + PY_MATRIX4X4_MARKER_MASK);
#if CROWN_DEBUG
		uintptr_t _vec3_marker;
		uintptr_t _quat_marker;
		uintptr_t _mat4_marker;
		Random _random;
#endif

		///
		PyWrapper();

		///
		~PyWrapper();

		///
		PyWrapper(const PyWrapper&) = delete;

		///
		PyWrapper& operator=(const PyWrapper&) = delete;

		void import_file(const char* name);

		/// Executes the string.
		void execute_string(const char* lua);

		/// Adds the function with the given @a name and @a func to the table @a module.
		void add_module_function(const char* module, const char* name, const char* func);

		/*/// Returns a new temporary Vector3.
		Vector3* next_vector3(const Vector3& v);

		/// Returns a new temporary Quaternion.
		Quaternion* next_quaternion(const Quaternion& q);

		/// Returns a new temporary Matrix4x4.
		Matrix4x4* next_matrix4x4(const Matrix4x4& m);

		/// Returns whether @a ptr is a temporary Vector3.
		bool is_vector3(const void* ptr);

		/// Returns whether @a ptr is a temporary Quaternion.
		bool is_quaternion(const void* ptr);

		/// Returns whether @a ptr is a temporary Matrix4x4.
		bool is_matrix4x4(const void* ptr);

		/// Returns the actual address of @a ptr if it is not stale,
		/// otherwise it generates a Lua error.
		Vector3* check_valid(const Vector3* ptr);

		/// Returns the actual address of @a ptr if it is not stale,
		/// otherwise it generates a Lua error.
		Quaternion* check_valid(const Quaternion* ptr);

		/// Returns the actual address of @a ptr if it is not stale,
		/// otherwise it generates a Lua error.
		Matrix4x4* check_valid(const Matrix4x4* ptr);

		/// Reloads (executes) all lua files that has been loaded since the program
		/// started.
		void reload();*/

		///
		void register_console_commands(ConsoleServer& cs);
	};

} // namespace crown

