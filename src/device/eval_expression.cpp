#include "eval_expression.h"
#include "python/py_evn.h"
#include <windows.h>
#include <iostream>
#include <strsafe.h>
#include "Python.h"


bool EvalExpr::quit = false;

static void error_msg()
{
	LPCTSTR lpszFunction = TEXT("GetProcessId");
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
}

void EvalExpr::eval(crown::PyWrapper* py_wrapper)
{
	HANDLE pipe;
	LPCSTR pipe_name = R"(\\.\pipe\crown_code)";

	// 创建命名管道
	pipe = CreateNamedPipe(
		pipe_name,             
		PIPE_ACCESS_DUPLEX,   // read write
		PIPE_TYPE_BYTE,       
		1,                    // one most instance
		0,                    // 
		0,                    // 
		NMPWAIT_WAIT_FOREVER, // 
		NULL);                // 

	if (INVALID_HANDLE_VALUE == pipe)
	{
		error_msg();
		return;
	}

	std::cout << "Named pipe server is waiting for connection..." << std::endl;

	// 等待客户端连接
	if (!ConnectNamedPipe(pipe, NULL)) {
		error_msg();
		CloseHandle(pipe);
		return;
	}

	std::cout << "Client connected. Waiting for messages..." << std::endl;

	while (!quit) {
		char buffer[2048];
		DWORD bytes;
		BOOL success = ReadFile(pipe, buffer, sizeof(buffer), &bytes, NULL);
		if (success && bytes > 0) {
			buffer[bytes] = '\0';
			PyGILState_STATE gstate;
			gstate = PyGILState_Ensure();
			py_wrapper->run_string(buffer);
			PyGILState_Release(gstate);
		}
		//else if (!success || bytesRead == 0) {
		//	error_msg();
		//	break;
		//}
	}

	CloseHandle(pipe);

	return;
}

