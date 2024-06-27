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
LOG_SYSTEM(PY, "python");
/// <summary>
/// redirect stdout stderr stdin
/// </summary>
PyObject* aview_write(PyObject* self, PyObject* args)
{
	const char* what;
	if (!PyArg_ParseTuple(args, "s", &what))
		return NULL;
	//printf("==%s==", what);
	printf("%s", what);
	return Py_BuildValue("");
}


PyObject* aview_flush(PyObject* self, PyObject* args)
{
	return Py_BuildValue("");
}


PyMethodDef aview_methods[] =
{
	{"write", aview_write, METH_VARARGS, "doc for write"},
	{"flush", aview_flush, METH_VARARGS, "doc for flush"},
	{0, 0, 0, 0} // sentinel
};


PyModuleDef aview_module =
{
	PyModuleDef_HEAD_INIT, // PyModuleDef_Base m_base;
	"aview",               // const char* m_name;
	"doc for aview",       // const char* m_doc;
	-1,                    // Py_ssize_t m_size;
	aview_methods,        // PyMethodDef *m_methods
	//  inquiry m_reload;  traverseproc m_traverse;  inquiry m_clear;  freefunc m_free;
};

PyMODINIT_FUNC PyInit_aview(void)
{
	PyObject* m = PyModule_Create(&aview_module);
	PySys_SetObject("stdout", m);
	PySys_SetObject("stderr", m);
	return m;
}


/// end redirect


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
	// setup python library path
	size_t len = std::strlen(argv[0]) + 1; // °üÀ¨¿ÕÖÕÖ¹·û
	std::vector<wchar_t> buffer(len);
	std::mbstowcs(buffer.data(), argv[0], len);
	std::wstring exe_file = std::wstring(buffer.data());
	auto pos = exe_file.find_last_of(L"/\\");
	CE_ASSERT(pos != std::string::npos, "executable file has no directory: %s", argv[0]);
	std::wstring dir = exe_file.substr(0, pos);
	std::wstring py_lib_dir = dir + L"/../../../3rdparty/python";

	PyConfig config;
	PyConfig_InitIsolatedConfig(&config);
	PyConfig_SetString(&config, &config.program_name, L"python");
	PyConfig_SetString(&config, &config.home, py_lib_dir.c_str());
	config.use_environment = 0;
	config.install_signal_handlers = 0;
	config.safe_path = 0;

	PyImport_AppendInittab("aview", PyInit_aview);
	PyStatus status = Py_InitializeFromConfig(&config);
	PyImport_ImportModule("aview");
	CE_ASSERT(Py_IsInitialized(), "python initialization failed: %s\n", status.err_msg);

	_local = _global = PyDict_New();
	_builtins = PyEval_GetBuiltins();
	PyDict_SetItemString(_global, "__builtins__", PyEval_GetBuiltins());

	PyConfig_Clear(&config);
}

void crown::PyWrapper::append_sys_path(const char* path)
{
	run_string("import sys");
	std::string p = std::string("sys.path.append(R\"") + path + "\")";
	run_string(p.c_str());
}

void crown::PyWrapper::import_file(const char* name)
{
	//std::string code = "import " + std::string(name);
	//execute_string(code.c_str());
	PyObject* py_name = PyUnicode_DecodeFSDefault(name);
	PyObject* py_module = PyImport_Import(py_name);

	if (nullptr == py_module)
	{
		PyErr_Print();
		Py_DECREF(py_name);
		return;
	}

	PyDict_SetItem(_global, py_name, py_module);
	PyDict_SetItem(_local, py_name, py_module);
	Py_DECREF(py_name);
	return;
}

void crown::PyWrapper::run_string(const char* code)
{
	/// reference 
	/// https://docs.python.org/zh-cn/3.7/faq/extending.html

	PyObject* py_code = Py_CompileString(code, "<string>", Py_file_input);
	if (nullptr != py_code)
	{
		PyObject* value = PyEval_EvalCode(py_code, _global, _local);
		Py_XDECREF(py_code);
		Py_XDECREF(value);
		if (PyErr_Occurred())
			PyErr_Print();
		return;
	}
	else/* if (PyErr_ExceptionMatches(PyExc_SyntaxError))*/ /// syntax error or E_EOF?
	{
		PyErr_Print();
	}
}

void crown::PyWrapper::invoke(const char* name)
{
	PyObject* func = query(name);
	if (func)
	{
		PyObject* result = PyObject_CallObject(func, nullptr);
		Py_XDECREF(result);
		Py_XDECREF(func);
	}
}

PyObject* crown::PyWrapper::query(const std::string& name)
{
	size_t start_pos = 0;
	size_t curr_pos = name.find_first_of('.');
	PyObject* func = nullptr;
	if (std::string::npos != curr_pos)
	{
		std::string module_name = name.substr(start_pos, curr_pos - start_pos);
		PyObject* py_module = PyDict_GetItemString(_global, module_name.c_str());
		if (nullptr != py_module)
		{
			std::string func_name;
			while (true)
			{
				start_pos = curr_pos + 1;
				curr_pos = name.find_first_of('.', start_pos);

				if (std::string::npos == curr_pos)
				{
					assert(py_module);
					auto fun_name = name.substr(start_pos);
					func = PyObject_GetAttrString(py_module, fun_name.c_str());
					break;
				}
				else
				{
					module_name = name.substr(start_pos, curr_pos - start_pos);
					py_module = PyObject_GetAttrString(py_module, module_name.c_str());
					if (nullptr == py_module)
						break;
				}
			}
		}
	}
	else  /// find in globals()
	{
		func = PyDict_GetItemString(_global, name.c_str());
		if (nullptr == func)
			func = PyDict_GetItemString(_builtins, name.c_str());
	}

	if (func == nullptr)
		logw(PY, "cannot found python function: %s", name);
	return func;
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
//
//static void do_REPL(LuaEnvironment* env, const char* lua)
//{
//	//lua_State* L = env->L;
//	//int status;
//
//	//lua_settop(L, 0);
//	//lua_pushstring(L, lua);
//	//if ((status = loadline(L)) != -1) { // This is never -1
//	//	if (status == LUA_OK) {
//	//		status = env->call(0, LUA_MULTRET);
//	//	}
//	//	if (status == LUA_OK)
//	//		l_print(L);
//	//	else
//	//		report(L, status);
//	//}
//	//lua_settop(L, 0); /* clear stack */
//	//return;
//}

void crown::PyWrapper::register_console_commands(ConsoleServer& cs)
{
	cs.register_message_type("script", console_command_script, this);
	cs.register_message_type("repl", console_command_REPL, this);
}

template <>
void crown::pack_params_inl(std::vector<PyObject*>& objs, const char* param)
{
	objs.push_back(Py_BuildValue("s", param));
}
