#pragma once

enum LOG_LEVEL {LOG_DEBUG=0,LOG_WARNING,LOG_ERROR};

#define MESSAGE_SIZE 256

#define SYSLOG_DIRECTORY(string) (logObject->GetInstance()->setDirectory(string))
#define SYSLOG_LEVEL(level) (logObject->GetInstance()->setLevel(level))
#define SYSLOG_LOG(type,level,stringFormat,...) (logObject->GetInstance()->Log(type,level,stringFormat,##__VA_ARGS__))
#define LOG SYSLOG_LOG

class CLog
{
public:
	static CLog *GetInstance();
	void Log(const WCHAR *szType,LOG_LEVEL LogLevel,const WCHAR *szStringFormat,...);
	void setDirectory(const WCHAR *directory);
	void setLevel(LOG_LEVEL logLevel);
private:
	CLog();
	static CLog *_log;
	LOG_LEVEL _logLevel;
	WCHAR _directory[MAX_PATH];
	int _logCount;
};

extern CLog *logObject;