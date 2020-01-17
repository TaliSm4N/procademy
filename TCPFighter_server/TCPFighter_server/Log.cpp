#define _WINSOCKAPI_
#include <Windows.h>
#include <ctime>
#include <iostream>

#include "Log.h"

int g_LogLevel = dfLOG_LEVEL_WARNING;
//int g_LogLevel = dfLOG_LEVEL_ERROR;
//int g_LogLevel = dfLOG_LEVEL_DEBUG;
WCHAR g_LogBuf[1024];

void Log(WCHAR *szString, int iLogLevel,bool showTime)
{
	time_t cur_time=time(NULL);
	tm cur_tm;

	FILE *f;
	errno_t err;
	err=fopen_s(&f,"./LogText.txt", "a+");

	if (err != 0)
	{
		wprintf(L"fopen error %d\n",err);
	}

	if (showTime)
	{
		localtime_s(&cur_tm, &cur_time);
		wprintf(L"[%02d/%02d/%02d %02d:%02d:%02d] ", cur_tm.tm_mon + 1, cur_tm.tm_mday, cur_tm.tm_year - 100, cur_tm.tm_hour, cur_tm.tm_min, cur_tm.tm_sec);
		if(f!=NULL)
			fwprintf_s(f, L"[%02d/%02d/%02d %02d:%02d:%02d] ", cur_tm.tm_mon + 1, cur_tm.tm_mday, cur_tm.tm_year - 100, cur_tm.tm_hour, cur_tm.tm_min, cur_tm.tm_sec);
	}
	wprintf(L"%s\n", szString);
	if (f != NULL)
	{
		fwprintf_s(f, L"%s\n", szString);
		fclose(f);
	}	
}