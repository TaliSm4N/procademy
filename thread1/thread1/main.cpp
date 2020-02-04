#include <iostream>
#include <Windows.h>
#include <process.h>
#include <conio.h>


HANDLE threadEvent;
HANDLE exitEvent;
HANDLE workerThread[20];
DWORD threadID[20];

unsigned __stdcall WorkerThreadProc(LPVOID lpThreadParameter);

int main()
{
	threadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	exitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);


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
			else if(key == 'q' || key == 'Q')
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

	return 0;
}

unsigned __stdcall WorkerThreadProc(LPVOID lpThreadParameter)
{
	DWORD state;
	HANDLE events[2] = { threadEvent,exitEvent };

	while (1)
	{
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
	}

	return 0;
}