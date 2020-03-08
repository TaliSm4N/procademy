#include <iostream>
#include <Windows.h>
#include <strsafe.h>
#include "Log.h"

void OuputFunction(LPTSTR _lpszRet, size_t _cchLen, LPCTSTR _lpszFormat, ...)
{
	TCHAR szContent[1024] = { 0, };

	if (NULL != _lpszFormat)
	{
		va_list vl;
		va_start(vl, _lpszFormat);
		StringCchVPrintf(szContent, sizeof(szContent) / sizeof(TCHAR), _lpszFormat, vl);
		va_end(vl);
	}

	if (NULL != _lpszRet)
		StringCchCopy(_lpszRet, _cchLen, szContent);
}

int main()
{
	TCHAR buf[100];
	OuputFunction(buf, 100,L"test %d %d %d", 1, 2, 3);

	wprintf(L"%s\n",buf);

	CLog *t;

	//t->GetInstance()->setLevel(LOG_LEVEL::LOG_WARNING);
	SYSLOG_LEVEL(LOG_LEVEL::LOG_WARNING);
	SYSLOG_DIRECTORY(L"..");

	//t->GetInstance()->Log(L"test", LOG_LEVEL::LOG_ERROR, L"%d test %c", 1, 'a');
	LOG(L"test", LOG_LEVEL::LOG_ERROR, L"%d test %c", 1, 'a');

	system("pause");
}