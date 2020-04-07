#include <iostream>
#include <Windows.h>
#include <process.h>
#include <conio.h>
#include "Profiler.h"


HANDLE threadEvent;
HANDLE exitEvent;
HANDLE workerThread[20];
DWORD threadID[20];

unsigned __stdcall WorkerThreadProc(LPVOID lpThreadParameter);

int main()
{
	threadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	exitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	void ProfileInit();

	for (int i = 0; i < 20; i++)
	{
		workerThread[i] = (HANDLE)_beginthreadex(NULL, 0, WorkerThreadProc, (void*)i, 0, (unsigned*)&threadID[i]);
	}

	while (1)
	{
		if (_kbhit())
		{
			WCHAR key = _getwch();

			if (key == 'x' || key == 'X')
			{
				SetEvent(threadEvent);
			}
			else if (key == 'q' || key == 'Q')
			{
				break;
			}
		}
	}

	std::cout << "exitEvent" << std::endl;
	SetEvent(exitEvent);


	DWORD state = WaitForMultipleObjects(20, workerThread, TRUE, INFINITE);

	if (state == WAIT_OBJECT_0)
	{
		std::cout << "All worker Thread is closed" << std::endl;

	}
	else
	{
		std::cout << "worker Thread close error" << std::endl;
	}

	ProfileDataOutText(L"threadTest");
	ProfileDataSumOutText(L"threadTestSum");

	return 0;
}

unsigned __stdcall WorkerThreadProc(LPVOID lpThreadParameter)
{
	DWORD state;
	HANDLE events[2] = { threadEvent,exitEvent };

	PRO_BEGIN(L"WORKER");

	while (1)
	{
		PRO_BEGIN(L"WHILE");
		state = WaitForMultipleObjects(2, events, false, INFINITE);

		if (state == WAIT_OBJECT_0)
		{
			std::cout << GetCurrentThreadId() << std::endl;
		}
		else if (state == WAIT_OBJECT_0 + 1)
		{
			std::cout << "exit thread event" << std::endl;
			break;
		}
		PRO_END(L"WHILE");
	}
	PRO_END(L"WORKER");

	return 0;
}
/*
#include <iostream>
#include <Windows.h>
#include <thread>
#include "Profiler.h"

void test1();
void test2();
void test3();
void test4();
void test5();

int main()
{
	for (int i = 0; i < 10; i++)
	{
		test1();
		test2();
		test3();
		test4();
		test5();
	}

	for (int i = 0; i < 5; i++)
	{
		test1();
		
		test3();
		
		test5();
	}

	for (int i = 0; i < 1; i++)
	{
		test3();
	}

	ProfileDataOutText(L"tt");
	system("pause");
}

void test1()
{
	PRO_BEGIN(L"TEST1");
	Sleep(55);
	PRO_END(L"TEST1");
}
void test2()
{
	PRO_BEGIN(L"TEST2");
	Sleep(0);
	PRO_END(L"TEST2");
}
void test3()
{
	PRO_BEGIN(L"TEST3");
	Sleep(22);
	PRO_END(L"TEST3");
}
void test4()
{
	PRO_BEGIN(L"TEST4");
	Sleep(100);
	PRO_END(L"TEST4");
}
void test5()
{
	PRO_BEGIN(L"TEST5");
	Sleep(300);
	PRO_END(L"TEST5");
}
*/