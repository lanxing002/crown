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

#include "Python.h"

using namespace crown;

int test()
{
	PyObject* pName, * pModule, * pFunc;
	PyObject* pArgs, * pValue;
	int i;

	//if (argc < 3) {
	//	fprintf(stderr, "Usage: call pythonfile funcname [args]\n");
	//	return 1;
	//}
	PyConfig config;
	PyConfig_SetString(&config, &config.home, L"/path/to/python");

	wchar_t wstr1[1024] = L"Python";
	PyConfig_InitIsolatedConfig(&config);
	config.use_environment = 0;
	config.program_name = wstr1;
	wchar_t wstr2[1024] = L"C:/Users/zele/Documents/code_ws/crown/3rdparty/python";
	config.home = wstr2;
	config.install_signal_handlers = 0;
	config.safe_path = 0;

	Py_InitializeFromConfig(&config);


	Py_Initialize();
	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path.append('C:/Users/zele/Documents/code_ws/Project2/x64/Debug')");

	//pName = PyUnicode_DecodeFSDefault(argv[1]);
	/* Error checking of pName left out */

	pModule = PyImport_Import(pName);
	Py_DECREF(pName);

	if (pModule != NULL) {
		pFunc = PyObject_GetAttrString(pModule, argv[2]);
		/* pFunc is a new reference */

		if (pFunc && PyCallable_Check(pFunc)) {
			pArgs = PyTuple_New(argc - 3);
			for (i = 0; i < argc - 3; ++i) {
				pValue = PyLong_FromLong(atoi(argv[i + 3]));
				if (!pValue) {
					Py_DECREF(pArgs);
					Py_DECREF(pModule);
					fprintf(stderr, "Cannot convert argument\n");
					return 1;
				}
				/* pValue reference stolen here: */
				PyTuple_SetItem(pArgs, i, pValue);
			}
			pValue = PyObject_CallObject(pFunc, pArgs);
			Py_DECREF(pArgs);
			if (pValue != NULL) {
				printf("Result of call: %ld\n", PyLong_AsLong(pValue));
				Py_DECREF(pValue);
			}
			else {
				Py_DECREF(pFunc);
				Py_DECREF(pModule);
				PyErr_Print();
				fprintf(stderr, "Call failed\n");
				return 1;
			}
		}
		else {
			if (PyErr_Occurred())
				PyErr_Print();
			fprintf(stderr, "Cannot find function \"%s\"\n", argv[2]);
		}
		Py_XDECREF(pFunc);
		Py_DECREF(pModule);
	}
	else {
		PyErr_Print();
		fprintf(stderr, "Failed to load \"%s\"\n", argv[1]);
		return 1;
	}
	if (Py_FinalizeEx() < 0) {
		return 120;
	}
	return 0;
}

crown::PyWrapper::PyWrapper()
	:_random(0)
{
	PyObject* py_name, * py_module, * py_func;
	PyObject* args, * value;

	PyConfig config;
	config.use_environment = 0;
	config.install_signal_handlers = 0;
	config.safe_path = 0;
	PyConfig_SetString(&config, &config.program_name, L"Python");
	PyConfig_SetString(&config, &config.home, L"C:/Users/zele/Documents/code_ws/crown/3rdparty/python"); //TODO:
	auto status = Py_InitializeFromConfig(&config);

	CE_ASSERT(PyStatus_Exception(status), "python initialization failed: %s\n", status.err_msg);
	PyConfig_Clear(&config);
}

crown::PyWrapper::~PyWrapper()
{
	int status = Py_FinalizeEx();
	CE_ASSERT(status < 0, "python finish failed !!");
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
	PyObject* value = PyRun_String(code, Py_file_input, PyEval_GetGlobals(), PyEval_GetLocals());
	if (nullptr == value)
		PyErr_Print();
}

void crown::PyWrapper::add_module_function(const char* module, const char* name, const char* func)
{
}

//static void console_command_script(ConsoleServer& /*cs*/, u32 /*client_id*/, const char* json, void* user_data)
//{
//	TempAllocator4096 ta;
//	JsonObject obj(ta);
//	DynamicString script(ta);
//
//	sjson::parse(obj, json);
//	sjson::parse_string(script, obj["script"]);
//
//	((LuaEnvironment*)user_data)->execute_string(script.c_str());
//}

static void console_command_REPL(ConsoleServer& cs, u32 client_id, const char* json, void* user_data)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	DynamicString script(ta);

	sjson::parse(obj, json);
	sjson::parse_string(script, obj["repl"]);

	do_REPL((LuaEnvironment*)user_data, script.c_str());
}

static void do_REPL(LuaEnvironment* env, const char* lua)
{
	lua_State* L = env->L;
	int status;

	lua_settop(L, 0);
	lua_pushstring(L, lua);
	if ((status = loadline(L)) != -1) { // This is never -1
		if (status == LUA_OK) {
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

void crown::PyWrapper::register_console_commands(ConsoleServer& cs)
{
	cs.register_message_type("script", console_command_script, this);
	cs.register_message_type("repl", console_command_REPL, this);
}
