#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>

#include "Profiler.h"



//PROFILE_SAMPLE profileSample[SAMPLE_MAX];
LARGE_INTEGER Frequency;

THREAD_SAMPLE threadSample[PRO_THREAD_MAX];


int sampleCur = -1;

DWORD g_tlsIndex = NULL;

void ProfileInit()
{
	QueryPerformanceFrequency(&Frequency);
	if (g_tlsIndex == NULL)
	{
		g_tlsIndex = TlsAlloc();
	}
}

PROFILE_SAMPLE *GetSample(THREAD_SAMPLE *t_sample, const WCHAR *szName, bool create = true)
{
	for (int i = 0; i < PRO_SAMPLE_MAX; i++)
	{
		if (t_sample->profileSample[i].lFlag)
		{
			if (wcscmp(t_sample->profileSample[i].szName, szName) == 0)
			{
				//profileSample[i].iCall++;
				//QueryPerformanceCounter(&profileSample[i].lStartTime);
				//break;

				return &t_sample->profileSample[i];
			}
		}
		else
		{
			if (create)
			{
				t_sample->profileSample[i].lFlag = true;
				wcscpy_s(t_sample->profileSample[i].szName, szName);

				//profileSample[i].iCall++;
				//QueryPerformanceCounter(&profileSample[i].lStartTime);
				//break;

				return &t_sample->profileSample[i];
			}
			else
				return NULL;
		}
	}

	return NULL;
}

void ProfileBegin(const WCHAR *szName)
{
	//static bool init = true;

	//if (init)
	THREAD_SAMPLE *t_sample = (THREAD_SAMPLE *)TlsGetValue(g_tlsIndex);
	PROFILE_SAMPLE *sample;

	//최초 실행시 초기화
	if (t_sample == NULL)
	{
		int index = InterlockedIncrement((LONG *)&sampleCur);
		t_sample = &threadSample[index];
		t_sample->threadID = GetCurrentThreadId();
		TlsSetValue(g_tlsIndex, t_sample);

		for (int i = 0; i < PRO_SAMPLE_MAX; i++)
		{
			t_sample->profileSample[i].lFlag = false;
			t_sample->profileSample[i].iTotalTime = 0;
			t_sample->profileSample[i].iMin[0] = _I64_MAX;
			t_sample->profileSample[i].iMin[1] = _I64_MAX;
			t_sample->profileSample[i].iMax[0] = 0;
			t_sample->profileSample[i].iMax[1] = 0;
			t_sample->profileSample[i].iCall = 0;
		}

	}

	sample = GetSample(t_sample, szName);

	QueryPerformanceCounter(&sample->lStartTime);
}

void ProfileEnd(const WCHAR *szName)
{
	LARGE_INTEGER endTime, profileTime;
	QueryPerformanceCounter(&endTime);
	THREAD_SAMPLE *t_sample = (THREAD_SAMPLE *)TlsGetValue(g_tlsIndex);
	PROFILE_SAMPLE *sample;

	if (t_sample == NULL)
		return;

	sample = GetSample(t_sample, szName, false);

	if (sample == NULL)
		return;


	profileTime.QuadPart = endTime.QuadPart - sample->lStartTime.QuadPart;
	sample->iTotalTime += profileTime.QuadPart;

	if (sample->iMax[1] < profileTime.QuadPart)
	{
		sample->iMax[0] = sample->iMax[1];
		sample->iMax[1] = profileTime.QuadPart;
	}

	if (sample->iMin[1] > profileTime.QuadPart)
	{
		sample->iMin[0] = sample->iMin[1];
		sample->iMin[1] = profileTime.QuadPart;
	}

	sample->iCall++;
}
void ProfileDataOutText(const WCHAR *szFileName)
{
	WCHAR path[MAX_PATH];

	FILE *f;
	SYSTEMTIME nowTime;
	errno_t err;
	GetLocalTime(&nowTime);
	swprintf_s(path, L"%4d-%02d-%02d %02d-%02d %s.txt", nowTime.wYear, nowTime.wMonth, nowTime.wDay, nowTime.wHour, nowTime.wMinute, szFileName);
	//swprintf_s(path, L"t.txt", path);

	err = _wfopen_s(&f, path, L"w");
	if (err != 0)
	{
		wprintf(L"fopen error %d\n", err);
		return;
	}

	fwprintf(f, L"----------------------------------------------------------------------------------------------------------\n");
	fwprintf(f, L"| ThreadID |          Name |                 Total |     Average |         Min |         Max |      Call |\n");
	fwprintf(f, L"----------------------------------------------------------------------------------------------------------\n");
	for (int i = 0; i < PRO_THREAD_MAX; i++)
	{
		if (threadSample[i].threadID == NULL)
			break;

		for (int j = 0; j < PRO_SAMPLE_MAX; j++)
		{
			PROFILE_SAMPLE *cur = &threadSample[i].profileSample[j];
			if (!cur->lFlag)
				break;
			fwprintf(f, L"|%9d |%14s |%20.2lf   |%10.4lf   | %10.4lf  | %10.4lf  | %10d|\n",
				threadSample[i].threadID,
				cur->szName,
				(double)cur->iTotalTime / 1000000,
				(double)cur->iTotalTime / 1000000 / cur->iCall,
				(double)cur->iMin[1] / 1000000,
				(double)cur->iMax[1] / 1000000,
				cur->iCall);

		}
		fwprintf(f, L"----------------------------------------------------------------------------------------------------------\n");
	}

	fwprintf(f, L"Unit : micro second\n");

	fclose(f);
}

void ProfileDataSumOutText(const WCHAR *szFileName)
{
	WCHAR path[MAX_PATH];

	PROFILE_SAMPLE temp[PRO_SAMPLE_MAX];
	int count[PRO_SAMPLE_MAX];

	FILE *f;
	SYSTEMTIME nowTime;
	errno_t err;
	GetLocalTime(&nowTime);
	swprintf_s(path, L"%4d-%02d-%02d %02d-%02d %s.txt", nowTime.wYear, nowTime.wMonth, nowTime.wDay, nowTime.wHour, nowTime.wMinute, szFileName);
	//swprintf_s(path, L"t.txt", path);

	err = _wfopen_s(&f, path, L"w");
	if (err != 0)
	{
		wprintf(L"fopen error %d\n", err);
		return;
	}

	for (int i = 0; i < PRO_THREAD_MAX; i++)
	{
		if (threadSample[i].threadID == NULL)
			break;

		for (int j = 0; j < PRO_SAMPLE_MAX; j++)
		{
			PROFILE_SAMPLE *cur = &threadSample[i].profileSample[j];
			if (!cur->lFlag)
				break;
			for (int index = 0; index < PRO_SAMPLE_MAX; index++)
			{

				if (!temp[index].lFlag)
				{
					temp[index].lFlag = true;
					temp[index].iMax[1] = cur->iMax[1];
					temp[index].iMin[1] = cur->iMin[1];
					temp[index].iTotalTime = cur->iTotalTime;
					temp[index].iCall = cur->iCall;
					wcscpy_s(temp[index].szName, (WCHAR *)cur->szName);
					break;

				}
				else if (wcscmp(cur->szName, temp[index].szName) == 0)
				{
					if (temp[index].iMax[1] < cur->iMax[1])
						temp[index].iMax[1] = cur->iMax[1];
					if (temp[index].iMin[1] > cur->iMin[1])
						temp[index].iMin[1] = cur->iMin[1];
					temp[index].iTotalTime += cur->iTotalTime;
					temp[index].iCall += cur->iCall;

					break;
				}
			}
		}
	}

	fwprintf(f, L"-----------------------------------------------------------------------------------------------\n");
	fwprintf(f, L"|          Name |                 Total |     Average |         Min |         Max |      Call |\n");
	fwprintf(f, L"-----------------------------------------------------------------------------------------------\n");


	for (int i = 0; i < PRO_SAMPLE_MAX; i++)
	{
		if (!temp[i].lFlag)
			break;
		fwprintf(f, L"|%14s |%20.2lf   |%10.4lf   | %10.4lf  | %10.4lf  | %10d|\n",
			temp[i].szName,
			(double)temp[i].iTotalTime / 1000000,
			(double)temp[i].iTotalTime / 1000000 / temp[i].iCall,
			(double)temp[i].iMin[1] / 1000000,
			(double)temp[i].iMax[1] / 1000000,
			temp[i].iCall);

	}
	fwprintf(f, L"-----------------------------------------------------------------------------------------------\n");

	fwprintf(f, L"Unit : micro second\n");

	fclose(f);
}

void ProfileReset(void)
{
	THREAD_SAMPLE *t_sample = (THREAD_SAMPLE *)TlsGetValue(g_tlsIndex);
	if (t_sample == NULL)
		return;
	for (int i = 0; i < PRO_SAMPLE_MAX; i++)
	{
		t_sample->profileSample[i].lFlag = false;
		t_sample->profileSample[i].iTotalTime = 0;
		t_sample->profileSample[i].iMin[0] = 0;
		t_sample->profileSample[i].iMin[1] = 0;
		t_sample->profileSample[i].iMax[0] = 0;
		t_sample->profileSample[i].iMax[1] = 0;
		t_sample->profileSample[i].iCall = 0;
	}
}

__int64 ProfileGetDataSumTotalTime(const WCHAR *szName)
{
	__int64 ret = 0;

	for (int i = 0; i < PRO_THREAD_MAX; i++)
	{
		if (threadSample[i].threadID == NULL)
			break;

		for (int j = 0; j < PRO_SAMPLE_MAX; j++)
		{
			PROFILE_SAMPLE *cur = &threadSample[i].profileSample[j];
			if (!cur->lFlag)
				break;

			if (wcscmp((WCHAR *)cur->szName, szName) == 0)
			{
				ret += cur->iTotalTime;
			}
		}
	}

	return ret;
}

__int64 ProfileGetDataSumMin(const WCHAR *szName)
{
	__int64 ret = 0;

	for (int i = 0; i < PRO_THREAD_MAX; i++)
	{
		if (threadSample[i].threadID == NULL)
			break;

		for (int j = 0; j < PRO_SAMPLE_MAX; j++)
		{
			PROFILE_SAMPLE *cur = &threadSample[i].profileSample[j];
			if (!cur->lFlag)
				break;

			if (wcscmp((WCHAR *)cur->szName, szName) == 0)
			{
				ret += cur->iMin[1];
			}
		}
	}

	return ret;
}

__int64 ProfileGetDataSumMax(const WCHAR *szName)
{
	__int64 ret = 0;

	for (int i = 0; i < PRO_THREAD_MAX; i++)
	{
		if (threadSample[i].threadID == NULL)
			break;

		for (int j = 0; j < PRO_SAMPLE_MAX; j++)
		{
			PROFILE_SAMPLE *cur = &threadSample[i].profileSample[j];
			if (!cur->lFlag)
				break;

			if (wcscmp((WCHAR *)cur->szName, szName) == 0)
			{
				ret += cur->iMax[1];
			}
		}
	}

	return ret;
}

__int64 ProfileGetDataSumCall(const WCHAR *szName)
{
	__int64 ret = 0;

	for (int i = 0; i < PRO_THREAD_MAX; i++)
	{
		if (threadSample[i].threadID == NULL)
			break;

		for (int j = 0; j < PRO_SAMPLE_MAX; j++)
		{
			PROFILE_SAMPLE *cur = &threadSample[i].profileSample[j];
			if (!cur->lFlag)
				break;

			if (wcscmp((WCHAR *)cur->szName, szName) == 0)
			{
				ret += cur->iCall;
			}
		}
	}

	return ret;
}

double ProfileGetDataSumAverage(const WCHAR *szName)
{
	__int64 tot = 0;
	__int64 call = 0;
	double ret = 0;

	for (int i = 0; i < PRO_THREAD_MAX; i++)
	{
		if (threadSample[i].threadID == NULL)
			break;

		for (int j = 0; j < PRO_SAMPLE_MAX; j++)
		{
			PROFILE_SAMPLE *cur = &threadSample[i].profileSample[j];
			if (!cur->lFlag)
				break;

			if (wcscmp((WCHAR *)cur->szName, szName) == 0)
			{
				tot += cur->iTotalTime;
				call += cur->iCall;
			}
		}
	}

	ret = (double)tot / call;

	return ret;
}