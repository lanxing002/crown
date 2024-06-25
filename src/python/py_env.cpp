#include "python/py_evn.h"
#include "config.h"
#include "core/error/error.h"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_id.inl"
#include "core/strings/string_stream.inl"
#include "device/device.h"
#include "device/log.h"
#include "resource/resource_manager.h"
#include <stdarg.h>
#include <string>
#include <vector>

#include "Python.h"

using namespace crown;

crown::PyWrapper::PyWrapper()
	:_random(0)
{
}

crown::PyWrapper::~PyWrapper()
{
	int status = Py_FinalizeEx();
	CE_ASSERT(status < 0, "python finish failed !!");
}

void crown::PyWrapper::init(const char** argv)
{
	PyObject* pName, * pModule, * pFunc;
	PyObject* pArgs, * pValue;

	PyConfig config;

	wchar_t wstr1[1024] = L"Python";
	PyConfig_InitIsolatedConfig(&config);
	config.use_environment = 0;
	config.program_name = wstr1;
	wchar_t wstr2[1024] = L"C:/Users/Lenovo/Documents/gly/crown/3rdparty/python";
	config.home = wstr2;
	config.install_signal_handlers = 0;
	config.safe_path = 0;

	Py_InitializeFromConfig(&config);


	Py_Initialize();
	PyRun_SimpleString("print('-AA----\n' * 10)");
	//Py_CompileString(TCHAR_TO_UTF8(InStr), TCHAR_TO_UTF8(InContext), InMode);
	//if (!PyCodeObj)
	//{
	//	return nullptr;
	//}
	//PyEval_EvalCode();
	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path.append('C:/Users/zele/Documents/code_ws/Project2/x64/Debug')");
	PyRun_SimpleString("print(1 + 2)");

	//pName = PyUnicode_DecodeFSDefault(argv[1]);
	/* Error checking of pName left out */

	//pModule = PyImport_Import(pName);
	//Py_DECREF(pName);

	//PyObject* py_name, * py_module, * py_func;
	//PyObject* args, * value;

	//size_t len = std::strlen(argv[0]) + 1; // °üÀ¨¿ÕÖÕÖ¹·û
	//std::vector<wchar_t> buffer(len);
	//std::mbstowcs(buffer.data(), argv[0], len);

	//// setup python library path
	//std::wstring exe_file = std::wstring(buffer.data());
	//auto pos = exe_file.find_last_of(L"/\\");
	//CE_ASSERT(pos != std::string::npos, "executable file has no directory: %s", argv[0]);
	//std::wstring dir = exe_file.substr(0, pos);
	//std::wstring py_lib_dir = dir + L"/../../../3rdparty/python";

	//PyConfig config;
	//PyConfig_InitIsolatedConfig(&config);
	//config.use_environment = 0;
	//config.install_signal_handlers = 0;
	//config.safe_path = 0;
	//PyConfig_SetString(&config, &config.program_name, L"Python");
	//PyConfig_SetString(&config, &config.home, py_lib_dir.c_str()); //TODO:
	//auto status = Py_InitializeFromConfig(&config);
	//Py_Initialize();

	////CE_ASSERT(PyStatus_Exception(status), "python initialization failed: %s\n", status.err_msg);
	//PyConfig_Clear(&config);

	//PyRun_String("print(1 + 2)", Py_single_input, PyEval_GetGlobals(), PyEval_GetLocals());
}

void crown::PyWrapper::append_sys_path(const char* path)
{

}

void crown::PyWrapper::import_file(const char* name)
{
	PyObject* py_name = PyUnicode_DecodeFSDefault(name);
	/* Error checking of pName left out */

	PyObject* py_module = PyImport_Import(py_name);
	if (nullptr == py_module)
		PyErr_Print();
}

void crown::PyWrapper::execute_string(const char* code)
{
	PyObject* py_code = Py_CompileString(code, "<string>", Py_single_input);
	if (!py_code)
	{
		return;
	}
	PyObject* value = PyEval_EvalCode(py_code, PyEval_GetGlobals(), PyEval_GetLocals());
	if (nullptr == value)
		PyErr_Print();
}

void crown::PyWrapper::add_module_function(const char* module, const char* name, const char* func)
{
}

static void console_command_script(ConsoleServer& /*cs*/, u32 /*client_id*/, const char* json, void* user_data)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	DynamicString script(ta);

	sjson::parse(obj, json);
	sjson::parse_string(script, obj["script"]);

	//((LuaEnvironment*)user_data)->execute_string(script.c_str());
}

static void console_command_REPL(ConsoleServer& cs, u32 client_id, const char* json, void* user_data)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	DynamicString script(ta);

	sjson::parse(obj, json);
	sjson::parse_string(script, obj["repl"]);

	//do_REPL((LuaEnvironment*)user_data, script.c_str());
}

static void do_REPL(LuaEnvironment* env, const char* lua)
{
	//lua_State* L = env->L;
	//int status;

	//lua_settop(L, 0);
	//lua_pushstring(L, lua);
	//if ((status = loadline(L)) != -1) { // This is never -1
	//	if (status == LUA_OK) {
	//		status = env->call(0, LUA_MULTRET);
	//	}
	//	if (status == LUA_OK)
	//		l_print(L);
	//	else
	//		report(L, status);
	//}
	//lua_settop(L, 0); /* clear stack */
	//return;
}

void crown::PyWrapper::register_console_commands(ConsoleServer& cs)
{
	cs.register_message_type("script", console_command_script, this);
	cs.register_message_type("repl", console_command_REPL, this);
}
