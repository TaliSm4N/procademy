
#include "CrashDump.h"

long CrashDump::_DumpCount = 0;

CrashDump::CrashDump()
{
	//_DumpCount = 0;

	_invalid_parameter_handler oldHandler, newHandler;
	newHandler = myInvalidParameterHandler;

	//_set_invalid_parameter_handler은 기존 핸들러를 return 값으로 가짐
	oldHandler = _set_invalid_parameter_handler(newHandler);
	_CrtSetReportMode(_CRT_WARN, 0);// crt 오류 메시지 표시 중단
	_CrtSetReportMode(_CRT_ASSERT, 0);// crt 오류 메시지 표시 중단
	_CrtSetReportMode(_CRT_ERROR, 0);// crt 오류 메시지 표시 중단

	_CrtSetReportHook(_custom_Report_hook);

	//pure virtual function called 에러 핸들러를 우회 시킴
	_set_purecall_handler(myPurecallHandler);

	_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);

	//해당하는 signal이 발생 시 crash
	signal(SIGABRT, signalHandler);
	signal(SIGINT, signalHandler);
	signal(SIGILL, signalHandler);
	signal(SIGFPE, signalHandler);
	signal(SIGSEGV, signalHandler);
	signal(SIGTERM, signalHandler);

	SetHandlerDump();
}

void CrashDump::Crash()
{
	int *p = nullptr;
	*p = 0;
}

LONG WINAPI CrashDump::MyExceptionFilter(__in PEXCEPTION_POINTERS pExceptionPointer)
{
	int iWorkingMemory = 0;
	SYSTEMTIME stNowTime;

	long DumpCount = InterlockedIncrement(&_DumpCount);

	//현재 날짜와 시간 얻어오기
	WCHAR filename[MAX_PATH];

	GetLocalTime(&stNowTime);
	wsprintf(filename, L"DUMP_%d%02d%02d_%02d.%02d.%02d_%d.dmp", stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond, DumpCount);

	wprintf(L"\n\n!!! Crash Error!!! %d.%d.%d / %d:%d:%d \n", stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond);
	wprintf(L"Now Save dump file...\n");

	HANDLE hDumpFile = ::CreateFile(filename,
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);

	if (hDumpFile != INVALID_HANDLE_VALUE)
	{
		_MINIDUMP_EXCEPTION_INFORMATION MinidumpExceptionInformation;

		MinidumpExceptionInformation.ThreadId = ::GetCurrentThreadId();
		MinidumpExceptionInformation.ExceptionPointers = pExceptionPointer;
		MinidumpExceptionInformation.ClientPointers = TRUE;

		MiniDumpWriteDump(GetCurrentProcess(),
			GetCurrentProcessId(),
			hDumpFile,
			MiniDumpWithFullMemory,
			&MinidumpExceptionInformation,
			NULL,
			NULL);
		CloseHandle(hDumpFile);

		wprintf(L"CrashDump Save Finish !");
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

LONG WINAPI CrashDump::RedirectedSetUnhandledExceptionFilter(EXCEPTION_POINTERS *exceptionInfo)
{
	MyExceptionFilter(exceptionInfo);
	return EXCEPTION_EXECUTE_HANDLER;
}

void CrashDump::SetHandlerDump()
{
	SetUnhandledExceptionFilter(MyExceptionFilter);
}

void CrashDump::myInvalidParameterHandler(const wchar_t *expression, const wchar_t *function, const wchar_t *filfile, unsigned int line, uintptr_t pReserved)
{
	Crash();
}

int CrashDump::_custom_Report_hook(int ireposttype, char *message, int *returnvalue)
{
	Crash();
	return true;
}

void CrashDump::myPurecallHandler(void)
{
	Crash();
}

void CrashDump::signalHandler(int Error)
{
	Crash();
}