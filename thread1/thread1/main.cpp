#include <iostream>
#include <Windows.h>
#include <process.h>
#include <conio.h>


HANDLE eventHandle[2];//threadEvent,exitEvent
HANDLE workerThread[20];
DWORD threadID[20];

unsigned __stdcall WorkerThreadProc(LPVOID lpThreadParameter);

int main()
{
	eventHandle[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
	eventHandle[1] = CreateEvent(NULL, TRUE, FALSE, NULL);


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
				SetEvent(eventHandle[0]);
			}
			else if(key == 'q' || key == 'Q')
			{
				break;
			}
		}
	}

	std::cout << "exitEvent" << std::endl;
	SetEvent(eventHandle[1]);
	

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

	while (1)
	{
		state = WaitForMultipleObjects(2, eventHandle, false, INFINITE);

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