
#include "py_evn.h"
#include "Python.h"
#include <vector>
using namespace crown;


////
////int test()
//// 
////{
////	PyObject* pName, * pModule, * pFunc;
////	PyObject* pArgs, * pValue;
////
////	PyConfig config;
////	PyConfig_SetString(&config, &config.home, L"/path/to/python");
////
////	wchar_t wstr1[1024] = L"Python";
////	PyConfig_InitIsolatedConfig(&config);
////	config.use_environment = 0;
////	config.program_name = wstr1;
////	wchar_t wstr2[1024] = L"C:/Users/zele/Documents/code_ws/crown/3rdparty/python";
////	config.home = wstr2;
////	config.install_signal_handlers = 0;
////	config.safe_path = 0;
////
////	Py_InitializeFromConfig(&config);
////
////
////	Py_Initialize();
////	PyRun_SimpleString("import sys");
////	PyRun_SimpleString("sys.path.append('C:/Users/zele/Documents/code_ws/Project2/x64/Debug')");
////
////	//pName = PyUnicode_DecodeFSDefault(argv[1]);
////	/* Error checking of pName left out */
////
////	pModule = PyImport_Import(pName);
////	Py_DECREF(pName);
////
////	if (pModule != NULL) {
////		pFunc = PyObject_GetAttrString(pModule, argv[2]);
////		/* pFunc is a new reference */
////
////		if (pFunc && PyCallable_Check(pFunc)) {
////			pArgs = PyTuple_New(argc - 3);
////			for (i = 0; i < argc - 3; ++i) {
////				pValue = PyLong_FromLong(atoi(argv[i + 3]));
////				if (!pValue) {
////					Py_DECREF(pArgs);
////					Py_DECREF(pModule);
////					fprintf(stderr, "Cannot convert argument\n");
////					return 1;
////				}
////				/* pValue reference stolen here: */
////				PyTuple_SetItem(pArgs, i, pValue);
////			}
////			pValue = PyObject_CallObject(pFunc, pArgs);
////			Py_DECREF(pArgs);
////			if (pValue != NULL) {
////				printf("Result of call: %ld\n", PyLong_AsLong(pValue));
////				Py_DECREF(pValue);
////			}
////			else {
////				Py_DECREF(pFunc);
////				Py_DECREF(pModule);
////				PyErr_Print();
////				fprintf(stderr, "Call failed\n");
////				return 1;
////			}
////		}
////		else {
////			if (PyErr_Occurred())
////				PyErr_Print();
////			fprintf(stderr, "Cannot find function \"%s\"\n", argv[2]);
////		}
////		Py_XDECREF(pFunc);
////		Py_DECREF(pModule);
////	}
////	else {
////		PyErr_Print();
////		fprintf(stderr, "Failed to load \"%s\"\n", argv[1]);
////		return 1;
////	}
////	if (Py_FinalizeEx() < 0) {
////		return 120;
////	}
////	return 0;
////}
