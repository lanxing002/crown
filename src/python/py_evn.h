#pragma once
#include "config.h"
#include "core/option.h"
#include "core/math/random.h"
#include "core/math/types.h"
#include "core/types.h"
#include "device/types.h"
#include "lua/lua_stack.h"
#include "resource/types.h"
#include "Python.h"


namespace crown
{

	struct PyWrapper
	{
#if CROWN_DEBUG
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

		///
		void init(const char** argv);

		/// <summary>
		void append_sys_path(const char* path);

		///
		void import_file(const char* name);

		/// Executes the string.
		void execute_string(const char* lua);

		/// Adds the function with the given @a name and @a func to the table @a module.
		void add_module_function(const char* module, const char* name, const char* func);

		//void call_global(const char* func, int narg, int nres);

		/// Reloads (executes) all lua files that has been loaded since the program
		/// started.
		void reload();

		///
		void register_console_commands(ConsoleServer& cs);

	private:
		PyObject* _local{ nullptr };
		PyObject* _global{nullptr};
	};

} // namespace crown

