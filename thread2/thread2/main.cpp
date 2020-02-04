#include <iostream>
#include <Windows.h>
#include <process.h>
#include <conio.h>
#include <list>
#include <ctime>

#define WORKER_CNT 3

unsigned __stdcall SaveThread(LPVOID lpThreadParameter);
unsigned __stdcall DeleteThread(LPVOID lpThreadParameter);
unsigned __stdcall PrintThread(LPVOID lpThreadParameter);
unsigned __stdcall WorkerThread(LPVOID lpThreadParameter);

//HANDLE eventHandle[3];//workerEvent, saveEvent,exitEvent
HANDLE workerEvent;
HANDLE saveEvent;
HANDLE exitEvent;

HANDLE threadHandle[3 + WORKER_CNT];
DWORD threadID[3 + WORKER_CNT];

SRWLOCK srwLock;

std::list<int> dataList;

int main()
{
	InitializeSRWLock(&srwLock);

	workerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	saveEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	exitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	threadHandle[0] = (HANDLE)_beginthreadex(NULL, 0, SaveThread, NULL, 0, (unsigned*)&threadID[0]);
	threadHandle[1] = (HANDLE)_beginthreadex(NULL, 0, DeleteThread, NULL, 0, (unsigned*)&threadID[1]);
	threadHandle[2] = (HANDLE)_beginthreadex(NULL, 0, PrintThread, NULL, 0, (unsigned*)&threadID[2]);

	for (int i = 3; i < 3 + WORKER_CNT; i++)
	{
		threadHandle[i] = (HANDLE)_beginthreadex(NULL, 0, WorkerThread, NULL, 0, (unsigned*)&threadID[i]);
	}

	while (1)
	{
		if (_kbhit())
		{
			WCHAR key = _getwch();

			if (key == 'w' || key == 'W')
			{
				SetEvent(workerEvent);
			}
			else if (key == 's' || key == 'S')
			{
				SetEvent(saveEvent);
			}
			else if (key == 'q' || key == 'Q')
			{
				SetEvent(exitEvent);
				break;
			}
		}
	}
	printf("exitEvent\n");
	
	DWORD state = WaitForMultipleObjects(3+WORKER_CNT, threadHandle, TRUE, INFINITE);

	std::cout << "All Thread is closed" << std::endl;

}

unsigned __stdcall SaveThread(LPVOID lpThreadParameter)
{
	DWORD state;
	HANDLE events[2];
	FILE *f;
	errno_t err;

	events[0] = saveEvent;
	events[1] = exitEvent;
	
	while (1)
	{
		state = WaitForMultipleObjects(2, events, FALSE, INFINITE);

		if (state == WAIT_OBJECT_0)
		{
			err = fopen_s(&f,"List.txt", "wt");

			if (err != 0)
			{
				printf("File Open error\n");
			}
			else
			{
				AcquireSRWLockShared(&srwLock);
				for (auto iter = dataList.begin(); iter != dataList.end(); iter++)
				{
					fprintf(f,"%03d-", *iter);
				}
				ReleaseSRWLockShared(&srwLock);
				fclose(f);
			}
			//ÀúÀå
		}
		else if(state == WAIT_OBJECT_0+1)
		{
			printf("Save Thread close Event\n");
			break;
		}
	}

	return 0;
}

unsigned __stdcall DeleteThread(LPVOID lpThreadParameter)
{
	DWORD state;

	while (1)
	{
		state = WaitForSingleObject(exitEvent, 333);

		if (state == WAIT_TIMEOUT)
		{
			if (!dataList.empty())
			{
				AcquireSRWLockExclusive(&srwLock);
				dataList.pop_back();
				ReleaseSRWLockExclusive(&srwLock);
			}
		}
		else if (state == WAIT_OBJECT_0)
		{
			printf("Delete Thread close Event\n");
			break;
		}
	}

	return 0;
}

unsigned __stdcall PrintThread(LPVOID lpThreadParameter)
{
	DWORD state;

	while (1)
	{
		state = WaitForSingleObject(exitEvent, 1000);

		if (state == WAIT_TIMEOUT)
		{
			AcquireSRWLockShared(&srwLock);
			for (auto iter = dataList.begin(); iter != dataList.end(); iter++)
			{
				printf("%03d-",*iter);
			}
			ReleaseSRWLockShared(&srwLock);
			printf("\n");
		}
		else if (state == WAIT_OBJECT_0)
		{
			printf("Print Thread close Event\n");
			break;
		}
	}

	return 0;
}

unsigned __stdcall WorkerThread(LPVOID lpThreadParameter)
{
	DWORD state;

	HANDLE events[2];

	static int wCnt = 0;

	events[0] = workerEvent;
	events[1] = exitEvent;

	while (1)
	{
		state = WaitForMultipleObjects(2, events, false, INFINITE);

		if (state == WAIT_OBJECT_0)
		{
			srand(time(NULL)*wCnt);
			wCnt++;

			AcquireSRWLockExclusive(&srwLock);
			dataList.push_front(rand()%1000);
			ReleaseSRWLockExclusive(&srwLock);
		}
		else if (state == WAIT_OBJECT_0 + 1)
		{
			printf("Worker Thread close Event\n");
			break;
		}
	}

	return 0;
}

