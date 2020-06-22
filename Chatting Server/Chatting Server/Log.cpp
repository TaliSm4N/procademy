#include <Windows.h>
#include <strsafe.h>
#include <ctime>
#include "Log.h"

CLog *CLog::_log = nullptr;

CLog *logObject;

CLog::CLog()
	:_logLevel(LOG_LEVEL::LOG_DEBUG),_logCount(1)
{
	wsprintf(_directory, L".");
}

CLog *CLog::GetInstance()
{
	if (_log == nullptr)
	{
		_log = new CLog();
	}

	return _log;
}

void CLog::Log(const WCHAR *szType, LOG_LEVEL LogLevel, const WCHAR *szStringFormat, ...)
{
	if (LogLevel < _logLevel)
	{
		return;
	}

	va_list va;

	time_t cur_time = time(NULL);
	tm cur_tm;

	WCHAR path[MAX_PATH];
	WCHAR message[MESSAGE_SIZE];
	FILE *f;
	errno_t err;
	//err = fopen_s(&f, "./LogText.txt", "w");

	localtime_s(&cur_tm, &cur_time);

	swprintf_s(path, L"%s\\%d%02d_%s.txt",_directory, cur_tm.tm_year + 1900, cur_tm.tm_mon + 1,szType);
	
	err = _wfopen_s(&f, path, L"a");

	if (err != 0)
	{
		wprintf(L"fopen error %d\n",err);
		return;
	}

	va_start(va, szStringFormat);

	StringCchVPrintf(message, MESSAGE_SIZE, szStringFormat, va);

	va_end(va);

	

	switch (LogLevel)
	{
	case LOG_LEVEL::LOG_DEBUG:
		fwprintf_s(f, L"[%d-%02d-%02d %02d:%02d:%02d / DEBUG   / %09d] %s\n", cur_tm.tm_year + 1900, cur_tm.tm_mon + 1, cur_tm.tm_mday, cur_tm.tm_hour, cur_tm.tm_min, cur_tm.tm_sec, _logCount++, message);
		break;
	case LOG_LEVEL::LOG_WARNING:
		fwprintf_s(f, L"[%d-%02d-%02d %02d:%02d:%02d / WARNING / %09d] %s\n", cur_tm.tm_year + 1900, cur_tm.tm_mon + 1, cur_tm.tm_mday, cur_tm.tm_hour, cur_tm.tm_min, cur_tm.tm_sec, _logCount++, message);
		break;
	case LOG_LEVEL::LOG_ERROR:
		fwprintf_s(f, L"[%d-%02d-%02d %02d:%02d:%02d / ERROR   / %09d] %s\n", cur_tm.tm_year + 1900, cur_tm.tm_mon + 1, cur_tm.tm_mday, cur_tm.tm_hour, cur_tm.tm_min, cur_tm.tm_sec, _logCount++, message);
		break;
	default:
		wprintf(L"Log Level failed\n");
		break;
	}


	

	fclose(f);

	//hResult = StringCchVPrintf(szInMessage, 256, szStringFormat, va);
}


void CLog::setDirectory(const WCHAR *directory)
{
	size_t len;
	StringCchLengthW(directory, MAX_PATH, &len);
	StringCchCopyNW(_directory, MAX_PATH, directory,len);
}

void CLog::setLevel(LOG_LEVEL logLevel)
{
	_logLevel = logLevel;
}