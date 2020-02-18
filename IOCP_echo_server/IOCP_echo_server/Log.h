#ifndef __LOG__
#define __LOG__
#define UNICODE


extern int g_LogLevel;
extern WCHAR g_LogBuf[1024];

#define _LOG(LogLevel, fmt, ...)				\
do{												\
	if(g_LogLevel<=LogLevel)					\
	{											\
		wsprintfW((LPWSTR)g_LogBuf,(LPCWSTR)fmt,##__VA_ARGS__);	\
		Log(g_LogBuf,LogLevel);					\
	}											\
} while (0)

#define _LOG_NOTIME(LogLevel, fmt, ...)				\
do{												\
	if(g_LogLevel<=LogLevel)					\
	{											\
		wsprintfW((LPWSTR)g_LogBuf,(LPCWSTR)fmt,##__VA_ARGS__);	\
		Log(g_LogBuf,LogLevel,false);					\
	}											\
} while (0)

#define dfLOG_LEVEL_DEBUG	0
#define dfLOG_LEVEL_WARNING	1
#define dfLOG_LEVEL_ERROR	2
#define dfLOG_LEVEL_ALWAYS	3



void Log(WCHAR *szString, int iLogLevel,bool showTime=true);
#endif // !__LOG__

