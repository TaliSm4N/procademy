#define _WINSOCKAPI_
#include <Windows.h>
#include <iostream>

#include "Log.h"

int g_LogLevel = dfLOG_LEVEL_DEBUG;
WCHAR g_LogBuf[1024];

void Log(WCHAR *szString, int iLogLevel)
{
	wprintf(L"%s\n", szString);
}