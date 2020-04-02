#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>

#include "Profiler.h"

#define SAMPLE_MAX 100

PROFILE_SAMPLE profileSample[SAMPLE_MAX];
LARGE_INTEGER Frequency;

void ProfileBegin(const WCHAR *szName)
{
	static bool init = true;

	if (init)
	{
		init = false;
		QueryPerformanceFrequency(&Frequency);
		for (int i = 0; i < SAMPLE_MAX; i++)
		{
			profileSample[i].lFlag = false;
			profileSample[i].iTotalTime = 0;
			profileSample[i].iMin[0] = _I64_MAX;
			profileSample[i].iMin[1] = _I64_MAX;
			profileSample[i].iMax[0] = 0;
			profileSample[i].iMax[1] = 0;
			profileSample[i].iCall = 0;
		}
	}

	for (int i = 0; i < SAMPLE_MAX; i++)
	{
		if (profileSample[i].lFlag)
		{
			if (wcscmp(profileSample[i].szName, szName) == 0)
			{
				//profileSample[i].iCall++;
				QueryPerformanceCounter(&profileSample[i].lStartTime);
				break;
			}
		}
		else
		{
			profileSample[i].lFlag = true;
			wcscpy_s(profileSample[i].szName, szName);

			//profileSample[i].iCall++;
			QueryPerformanceCounter(&profileSample[i].lStartTime);
			break;
		}
	}
}

void ProfileEnd(const WCHAR *szName)
{
	for (int i = 0; i < SAMPLE_MAX; i++)
	{
		if (profileSample[i].lFlag)
		{
			if (wcscmp(profileSample[i].szName, szName) == 0)
			{
				LARGE_INTEGER endTime,profileTime;
				QueryPerformanceCounter(&endTime);
				profileTime.QuadPart = endTime.QuadPart - profileSample[i].lStartTime.QuadPart;
				profileSample[i].iTotalTime += profileTime.QuadPart;

				if (profileSample[i].iMax[1] < profileTime.QuadPart)
				{
					profileSample[i].iMax[0] = profileSample[i].iMax[1];
					profileSample[i].iMax[1] = profileTime.QuadPart;
				}

				if (profileSample[i].iMin[1] > profileTime.QuadPart)
				{
					profileSample[i].iMin[0] = profileSample[i].iMin[1];
					profileSample[i].iMin[1] = profileTime.QuadPart;
				}

				profileSample[i].iCall++;
				break;
			}
		}
		else
		{
			break;
		}
	}
}
void ProfileDataOutText(const WCHAR *szFileName)
{
	WCHAR path[MAX_PATH];

	FILE *f;
	SYSTEMTIME nowTime;
	errno_t err;
	GetLocalTime(&nowTime);
	swprintf_s(path, L"%4d-%02d-%02d %02d-%02d %s.txt",nowTime.wYear, nowTime.wMonth, nowTime.wDay, nowTime.wHour, nowTime.wMinute,szFileName);
	//swprintf_s(path, L"t.txt", path);

	err = _wfopen_s(&f, path, L"w");
	if (err != 0)
	{
		wprintf(L"fopen error %d\n", err);
		return;
	}

	fwprintf(f, L"----------------------------------------------------------------------\n");
	fwprintf(f, L"|          Name |     Average |         Min |         Max |      Call |\n");
	fwprintf(f, L"----------------------------------------------------------------------\n");
	for (int i = 0; i < SAMPLE_MAX; i++)
	{
		if (!profileSample[i].lFlag)
			break;

		fwprintf(f, L"|%14s |%10.4lf   | %10.4lf  | %10.4lf  | %10d|\n",
			profileSample[i].szName, 
			(double)profileSample[i].iTotalTime / 1000000 / profileSample[i].iCall, 
			(double)profileSample[i].iMin[1] / 1000000, 
			(double)profileSample[i].iMax[1] / 1000000, 
			profileSample[i].iCall);
	}
	fwprintf(f, L"----------------------------------------------------------------------\n");
	fwprintf(f, L"Unit : micro second\n");

	fclose(f);
}
void ProfileReset(void)
{
	for (int i = 0; i < SAMPLE_MAX; i++)
	{
		profileSample[i].lFlag = false;
		profileSample[i].iTotalTime = 0;
		profileSample[i].iMin[0] = 0;
		profileSample[i].iMin[1] = 0;
		profileSample[i].iMax[0] = 0;
		profileSample[i].iMax[1] = 0;
		profileSample[i].iCall = 0;
	}
}