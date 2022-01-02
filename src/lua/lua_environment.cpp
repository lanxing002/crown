/*
 * Copyright (c) 2012-2022 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "config.h"
#include "core/error/error.h"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_stream.inl"
#include "device/device.h"
#include "device/log.h"
#include "lua/lua_environment.h"
#include "lua/lua_stack.inl"
#include "resource/lua_resource.h"
#include "resource/resource_manager.h"
#include <lua.hpp>
#include <stdarg.h>

LOG_SYSTEM(LUA, "lua")

namespace crown
{
extern void load_api(LuaEnvironment& env);

static int luaB_print(lua_State* L)
{
	TempAllocator2048 ta;
	StringStream ss(ta);

	int n = lua_gettop(L); /* number of arguments */
	lua_getglobal(L, "tostring");
	for (int i = 1; i <= n; ++i)
	{
		const char *s;
		lua_pushvalue(L, -1); /* function to be called */
		lua_pushvalue(L, i);  /* value to print */
		lua_call(L, 1, 1);
		s = lua_tostring(L, -1); /* get result */
		if (s == NULL)
			return luaL_error(L, LUA_QL("tostring") " must return a string to " LUA_QL("print"));

		if (i > 1)
			ss << "\t";
		ss << s;
		lua_pop(L, 1); /* pop result */
	}

	logi(LUA, string_stream::c_str(ss));
	return 0;
}

static int msghandler(lua_State* L) {
	const char *msg = lua_tostring(L, 1);
	if (msg == NULL) {  /* is error object not a string? */
		if (luaL_callmeta(L, 1, "__tostring") &&  /* does it have a metamethod */
			lua_type(L, -1) == LUA_TSTRING)  /* that produces a string? */
			return 1;  /* that is the message */
		else
			msg = lua_pushfstring(L, "(error object is a %s value)",
								 luaL_typename(L, 1));
	}
	luaL_traceback(L, L, msg, 1);  /* append a standard traceback */
	return 1;  /* return the traceback */
}

/*
** Prints (calling the Lua 'print' function) any values on the stack
*/
static void l_print (lua_State *L) {
	int n = lua_gettop(L);
	if (n > 0) {  /* any result to be printed? */
		luaL_checkstack(L, LUA_MINSTACK, "too many results to print");
		lua_getglobal(L, "print");
		lua_insert(L, 1);
		if (lua_pcall(L, n, 0, 0) != LUA_OK)
			loge(LUA, lua_pushfstring(L, "error calling 'print' (%s)", lua_tostring(L, -1)));
	}
}

/*
** Try to compile line on the stack as 'return <line>;'; on return, stack
** has either compiled chunk or original line (if compilation failed).
*/
static int addreturn (lua_State *L) {
	const char *line = lua_tostring(L, -1);  /* original line */
	const char *retline = lua_pushfstring(L, "return %s;", line);
	int status = luaL_loadbuffer(L, retline, strlen(retline), "=stdin");
	if (status == LUA_OK)
		lua_remove(L, -2);  /* remove modified line */
	else
		lua_pop(L, 2);  /* pop result from 'luaL_loadbuffer' and modified line */
	return status;
}

/*
** Read a line and try to load (compile) it first as an expression (by
** adding "return " in front of it) and second as a statement. Return
** the final status of load/call with the resulting function (if any)
** in the top of the stack.
*/
static int loadline (lua_State *L) {
	int status;
	if ((status = addreturn(L)) != LUA_OK) { /* 'return ...' did not work? */
		// status = multiline(L);  /* try as command, maybe with continuation lines */
		size_t len;
		const char *line = lua_tolstring(L, 1, &len);  /* get what it has */
		status = luaL_loadbuffer(L, line, len, "=stdin");  /* try it */
	}
	lua_remove(L, 1);  /* remove line from the stack */
	lua_assert(lua_gettop(L) == 1);
	return status;
}

/*
** Check whether 'status' is not OK and, if so, prints the error
** message on the top of the stack. It assumes that the error object
** is a string, as it was either generated by Lua or by 'msghandler'.
*/
int report (lua_State *L, int status) {
	if (status != LUA_OK) {
		const char *msg = lua_tostring(L, -1);
		loge(LUA, msg);
		lua_pop(L, 1);  /* remove message */
	}
	return status;
}

static int loader(lua_State* L)
{
	LuaStack stack(L);
	int status;

	const LuaResource* lr = (LuaResource*)device()->_resource_manager->get(RESOURCE_TYPE_SCRIPT, stack.get_resource_name(1));
	status = luaL_loadbuffer(L, lua_resource::program(lr), lr->size, "");
	if (status != LUA_OK)
	{
		report(L, status);
		device()->pause();
	}
	return 1;
}

#if CROWN_DEBUG
static int require_internal(lua_State* L)
{
	const char* module_name = lua_tostring(L, 1);
	bool already_loaded = false;

	lua_getfield(L, LUA_REGISTRYINDEX, "_LOADED");
	lua_getfield(L, -1, module_name);
	already_loaded = lua_toboolean(L, -1);
	lua_pop(L, 2);

	lua_getglobal(L, "original_require");
	lua_pushvalue(L, -2);
	lua_remove(L, -3);
	lua_call(L, 1, 1);

	lua_getfield(L, LUA_REGISTRYINDEX, "_LOADED");
	lua_getfield(L, 2, module_name);
	if (lua_toboolean(L, -1) && !already_loaded)
	{
		lua_pop(L, 2);
		lua_getglobal(L, "package");
		lua_getfield(L, -1, "load_order");
		lua_pushstring(L, module_name);
		lua_rawseti(L, -2, int(lua_objlen(L, -2) + 1));
		lua_pop(L, 2); // Pop "package.load_order" and "package"
	}

	return 1;
}
#endif

LuaEnvironment::LuaEnvironment()
	: L(NULL)
	, _num_vec3(0)
	, _num_quat(0)
	, _num_mat4(0)
#if CROWN_DEBUG
	, _vec3_marker(0)
	, _quat_marker(0)
	, _mat4_marker(0)
	, _random(0)
#endif
{
#if CROWN_DEBUG
	// Initialize temporaries markers with random values.
	_random._seed = (s32)guid::new_guid().data1;
	reset_temporaries();
#endif

	L = luaL_newstate();
	CE_ASSERT(L, "Unable to create lua state");
}

LuaEnvironment::~LuaEnvironment()
{
	lua_close(L);
}

void LuaEnvironment::load_libs()
{
	lua_gc(L, LUA_GCSTOP, 0);

	// Open default libraries
	lua_pushcfunction(L, luaopen_base);
	lua_pushstring(L, "");
	lua_call(L, 1, 0);
	lua_pushcfunction(L, luaopen_package);
	lua_pushstring(L, LUA_LOADLIBNAME);
	lua_call(L, 1, 0);
	lua_pushcfunction(L, luaopen_table);
	lua_pushstring(L, LUA_TABLIBNAME);
	lua_call(L, 1, 0);
	lua_pushcfunction(L, luaopen_string);
	lua_pushstring(L, LUA_STRLIBNAME);
	lua_call(L, 1, 0);
	lua_pushcfunction(L, luaopen_math);
	lua_pushstring(L, LUA_MATHLIBNAME);
	lua_call(L, 1, 0);
	lua_pushcfunction(L, luaopen_debug);
	lua_pushstring(L, LUA_DBLIBNAME);
	lua_call(L, 1, 0);
	lua_pushcfunction(L, luaopen_bit);
	lua_pushstring(L, LUA_BITLIBNAME);
	lua_call(L, 1, 0);
	lua_pushcfunction(L, luaopen_jit);
	lua_pushstring(L, LUA_JITLIBNAME);
	lua_call(L, 1, 0);

	// Override print to redirect output to logging system
	add_module_function("_G", "print", luaB_print);

	// Register crown libraries
	load_api(*this);

	// Register custom loader
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "loaders");
	lua_pushcfunction(L, loader);
	lua_rawseti(L, -2, 1); // package.loaders[1] = loader
	lua_pushnil(L);
	lua_rawseti(L, -2, 2); // package.loaders[2] = nil
	lua_pushnil(L);
	lua_rawseti(L, -2, 3); // package.loaders[3] = nil
	lua_pushnil(L);
	lua_rawseti(L, -2, 4); // package.loaders[4] = nil
	lua_pop(L, 1);         // pop "package.loaders"
#if CROWN_DEBUG
	// Create new empty table to store package load order
	lua_newtable(L);
	lua_setfield(L, -2, "load_order"); // package.load_order = {}

	// Override require() to keep track of libraries' load order
	lua_getglobal(L, "require");
	lua_setglobal(L, "original_require");
	lua_pushcclosure(L, require_internal, 0); // pass original require() to new require as upvalue
	lua_setglobal(L, "require");
#endif
	lua_pop(L, 1);         // pop "package"

	// Create metatable for lightuserdata
	lua_pushlightuserdata(L, 0);
	lua_getfield(L, LUA_REGISTRYINDEX, "Lightuserdata_mt");
	lua_setmetatable(L, -2);
	lua_pop(L, 1);

	// Ensure stack is clean
	CE_ASSERT(lua_gettop(L) == 0, "Stack not clean");

	lua_gc(L, LUA_GCRESTART, 0);
}

void LuaEnvironment::do_file(const char* name)
{
	lua_getglobal(L, "dofile");
	lua_pushstring(L, name);
	this->call(1, 0);
}

void LuaEnvironment::require(const char* name)
{
	lua_getglobal(L, "require");
	lua_pushstring(L, name);
	this->call(1, 0);
}

LuaStack LuaEnvironment::execute(const LuaResource* lr, int nres)
{
	LuaStack stack(L);
	int status;

	status = luaL_loadbuffer(L, lua_resource::program(lr), lr->size, "<unknown>");
	if (status == LUA_OK)
		status = this->call(0, nres);
	if (status != LUA_OK)
	{
		report(L, status);
		device()->pause();
	}

	return stack;
}

LuaStack LuaEnvironment::execute_string(const char* s)
{
	LuaStack stack(L);
	int status;

	status = luaL_loadstring(L, s);
	if (status == LUA_OK)
		status = this->call(0, 0);
	if (status != LUA_OK)
	{
		report(L, status);
		device()->pause();
	}

	CE_ASSERT(lua_gettop(L) == 0, "Stack not clean");
	return stack;
}

void LuaEnvironment::add_module_function(const char* module, const char* name, const lua_CFunction func)
{
	luaL_Reg entry[2];
	entry[0].name = name;
	entry[0].func = func;
	entry[1].name = NULL;
	entry[1].func = NULL;

	luaL_register(L, module, entry);
	lua_pop(L, 1);
}

void LuaEnvironment::add_module_function(const char* module, const char* name, const char* func)
{
	// Create module if it does not exist
	luaL_Reg entry;
	entry.name = NULL;
	entry.func = NULL;
	luaL_register(L, module, &entry);
	lua_pop(L, 1);

	lua_getglobal(L, module);
	lua_getglobal(L, func);
	lua_setfield(L, -2, name);
	lua_setglobal(L, module);
}

void LuaEnvironment::add_module_metafunction(const char* module, const char* name, const lua_CFunction func)
{
	// Create module if it does not exist
	luaL_Reg entry[2];
	entry[0].name = NULL;
	entry[0].func = NULL;
	luaL_register(L, module, entry);
	lua_pop(L, 1);

	luaL_newmetatable(L, module);
	if (func)
	{
		entry[0].name = name;
		entry[0].func = func;
		entry[1].name = NULL;
		entry[1].func = NULL;
		luaL_register(L, NULL, entry);
	}
	else
	{
		lua_pushstring(L, name);
		lua_pushvalue(L, -2);
		lua_settable(L, -3);
	}

	lua_getglobal(L, module);
	lua_pushvalue(L, -2);
	lua_setmetatable(L, -2);
	lua_pop(L, -1);
}

int LuaEnvironment::call(int narg, int nres)
{
	int status;
	int base = lua_gettop(L) - narg;  /* function index */
	lua_pushcfunction(L, msghandler);  /* push message handler */
	lua_insert(L, base);  /* put it under function and args */
	status = lua_pcall(L, narg, nres, base);
	lua_remove(L, base);  /* remove message handler from the stack */
	return status;
}

void LuaEnvironment::call_global(const char* func, int narg, int nres)
{
	int status;
	CE_ENSURE(NULL != func);

	lua_getglobal(L, func);
	lua_insert(L, 1); // Move func to the top of stack
	status = call(narg, nres);
	if (status != LUA_OK)
	{
		report(L, status);
		device()->pause();
	}

	CE_ASSERT(lua_gettop(L) == 0, "Stack not clean");
}

LuaStack LuaEnvironment::get_global(const char* global)
{
	LuaStack stack(L);
	lua_getglobal(L, global);
	return stack;
}

Vector3* LuaEnvironment::next_vector3(const Vector3& v)
{
	CE_ASSERT(_num_vec3 < LUA_MAX_VECTOR3, "Maximum number of Vector3 reached");

	Vector3* vec3 = &(_vec3[_num_vec3++] = v);
	uintptr_t ptr = (uintptr_t)(void*)vec3;
#if CROWN_DEBUG
	ptr |= _vec3_marker;
#endif
	return (Vector3*)ptr;
}

Quaternion* LuaEnvironment::next_quaternion(const Quaternion& q)
{
	CE_ASSERT(_num_quat < LUA_MAX_QUATERNION, "Maximum number of Quaternion reached");

	Quaternion* quat = &(_quat[_num_quat++] = q);
	uintptr_t ptr = (uintptr_t)(void*)quat;
#if CROWN_DEBUG
	ptr |= _quat_marker;
#endif
	return (Quaternion*)ptr;
}

Matrix4x4* LuaEnvironment::next_matrix4x4(const Matrix4x4& m)
{
	CE_ASSERT(_num_mat4 < LUA_MAX_MATRIX4X4, "Maximum number of Matrix4x4 reached");

	Matrix4x4* mat4 = &(_mat4[_num_mat4++] = m);
	uintptr_t ptr = (uintptr_t)(void*)mat4;
#if CROWN_DEBUG
	ptr |= _mat4_marker;
#endif
	return (Matrix4x4*)ptr;
}

bool LuaEnvironment::is_vector3(const void* ptr)
{
	return ptr >= &_vec3[0]
		&& ptr <= &_vec3[LUA_MAX_VECTOR3 - 1];
}

bool LuaEnvironment::is_quaternion(const void* ptr)
{
	return ptr >= &_quat[0]
		&& ptr <= &_quat[LUA_MAX_QUATERNION - 1];
}

bool LuaEnvironment::is_matrix4x4(const void* ptr)
{
	return ptr >= &_mat4[0]
		&& ptr <= &_mat4[LUA_MAX_MATRIX4X4 - 1];
}

#if CROWN_DEBUG
Vector3* LuaEnvironment::check_valid(const Vector3* ptr)
{
	LUA_ASSERT(((uintptr_t)ptr & LUA_VECTOR3_MARKER_MASK) >> LUA_VECTOR3_MARKER_SHIFT == _vec3_marker
		, LuaStack(L)
		, "Stale Vector3 ptr: %p, expected marker: %p"
		, ptr
		, _vec3_marker
		);
	return (Vector3*)(uintptr_t(ptr) & ~LUA_VECTOR3_MARKER_MASK);
}

Quaternion* LuaEnvironment::check_valid(const Quaternion* ptr)
{
	LUA_ASSERT(((uintptr_t)ptr & LUA_QUATERNION_MARKER_MASK) >> LUA_QUATERNION_MARKER_SHIFT == _quat_marker
		, LuaStack(L)
		, "Stale Quaternion ptr: %p, expected marker: %p"
		, ptr
		, _quat_marker
		);
	return (Quaternion*)(uintptr_t(ptr) & ~LUA_QUATERNION_MARKER_MASK);
}

Matrix4x4* LuaEnvironment::check_valid(const Matrix4x4* ptr)
{
	LUA_ASSERT(((uintptr_t)ptr & LUA_MATRIX4X4_MARKER_MASK) >> LUA_MATRIX4X4_MARKER_SHIFT == _mat4_marker
		, LuaStack(L)
		, "Stale Matrix4x4 ptr: %p, expected marker: %p"
		, ptr
		, _mat4_marker
		);
	return (Matrix4x4*)(uintptr_t(ptr) & ~LUA_MATRIX4X4_MARKER_MASK);
}
#endif // CROWN_DEBUG

void LuaEnvironment::reload()
{
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "load_order");
	for (size_t i = 1, n = lua_objlen(L, -1); i < n+1; ++i)
	{
		lua_rawgeti(L, -1, (int)i);
		logi(LUA, "reloading: %s", lua_tostring(L, -1));

		LuaStack stack(L);
		StringId64 name = stack.get_resource_name(-1);
		this->execute((const LuaResource*)device()->_resource_manager->get(RESOURCE_TYPE_SCRIPT, name), 0);

		lua_pop(L, 1);
	}
	lua_pop(L, 2);
}

void LuaEnvironment::temp_count(u32& num_vec3, u32& num_quat, u32& num_mat4)
{
	num_vec3 = _num_vec3;
	num_quat = _num_quat;
	num_mat4 = _num_mat4;
}

void LuaEnvironment::set_temp_count(u32 num_vec3, u32 num_quat, u32 num_mat4)
{
	_num_vec3 = num_vec3;
	_num_quat = num_quat;
	_num_mat4 = num_mat4;
}

void LuaEnvironment::reset_temporaries()
{
	_num_vec3 = 0;
	_num_quat = 0;
	_num_mat4 = 0;

#if CROWN_DEBUG
	_vec3_marker = (uintptr_t)_random.integer(1 + LUA_VECTOR3_MARKER_MASK);
	_quat_marker = (uintptr_t)_random.integer(1 + LUA_QUATERNION_MARKER_MASK);
	_mat4_marker = (uintptr_t)_random.integer(1 + LUA_MATRIX4X4_MARKER_MASK);
#endif
}

static void console_command_script(ConsoleServer& /*cs*/, u32 /*client_id*/, const char* json, void* user_data)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	DynamicString script(ta);

	sjson::parse(obj, json);
	sjson::parse_string(script, obj["script"]);

	((LuaEnvironment*)user_data)->execute_string(script.c_str());
}

static void do_REPL(LuaEnvironment* env, const char* lua)
{
	lua_State* L = env->L;
	int status;

	lua_settop(L, 0);
	lua_pushstring(L, lua);
	if ((status = loadline(L)) != -1) // This is never -1
	{
		if (status == LUA_OK)
		{
			status = env->call(0, LUA_MULTRET);
		}
		if (status == LUA_OK)
			l_print(L);
		else
			report(L, status);
	}
	lua_settop(L, 0); /* clear stack */
	return;
}

static void console_command_REPL(ConsoleServer& /*cs*/, u32 /*client_id*/, const char* json, void* user_data)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	DynamicString script(ta);

	sjson::parse(obj, json);
	sjson::parse_string(script, obj["repl"]);

	do_REPL((LuaEnvironment*)user_data, script.c_str());
}

void LuaEnvironment::register_console_commands(ConsoleServer& cs)
{
	cs.register_message_type("script", console_command_script, this);
	cs.register_message_type("repl", console_command_REPL, this);
}

} // namespace crown
