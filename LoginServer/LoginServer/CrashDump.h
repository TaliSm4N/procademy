#ifndef __LUM0_CRASH_DUMP__
#define __LUM0_CRASH_DUMP__

#include <Windows.h>// 환인 후 삭제
#pragma comment(lib, "DbgHelp.Lib")
#include <dbghelp.h>
#include <Psapi.h>
#include <iostream>
#include <signal.h>

class CrashDump
{
public: 
	CrashDump();

	static void Crash();

	static LONG WINAPI MyExceptionFilter(__in PEXCEPTION_POINTERS pExceptionPointer);
	static LONG WINAPI RedirectedSetUnhandledExceptionFilter(EXCEPTION_POINTERS *exceptionInfo);

	static void SetHandlerDump();

	//invalid Parameter handler
	static void myInvalidParameterHandler(const wchar_t *expression, const wchar_t *function, const wchar_t *filfile, unsigned int line, uintptr_t pReserved);

	static int _custom_Report_hook(int ireposttype, char *message, int *returnvalue);

	static void myPurecallHandler(void);

	static void signalHandler(int Error);
private:
	static long _DumpCount;
};

#endif