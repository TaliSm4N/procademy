#include <iostream>
#include <Windows.h>
#include <process.h>
#include <list>
#include <ctime>
#include <conio.h>
#include "RingBuffer.h"


#define DELAY_TIME 0
#define WORKER_CNT 20

#define dfTYPE_ADD_STR 0
#define dfTYPE_DEL_STR 1
#define dfTYPE_PRINT_LIST 2
#define dfTYPE_QUIT 3 


struct st_MSG_HEAD
{
	short shType;
	short shStrLen;
};


std::wstring g_str = L"PROCADEMY";
std::list<std::wstring> g_List;

SRWLOCK listLock;

RingBuffer g_msgQ(40000);

HANDLE wakeEvent;
HANDLE threadHandle[WORKER_CNT];
DWORD threadID[WORKER_CNT];


unsigned __stdcall WorkerThread(LPVOID lpThreadParameter);
void addStr(std::wstring &str);
void delStr();
void printList();

int main()
{
	bool loop=true;
	st_MSG_HEAD header;
	InitializeSRWLock(&listLock);
	srand(time(NULL));
	wakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	for (int i = 0; i < WORKER_CNT; i++)
	{
		threadHandle[i]=(HANDLE)_beginthreadex(NULL, 0, WorkerThread, NULL, 0, (unsigned*)&threadID[i]);
	}

	while (loop)
	{
		header.shType = rand() % 3;
		header.shStrLen = rand() % g_str.length()+1;

		if (_kbhit())
		{
			WCHAR key = _getch();

			if (key == ' ')
			{
				header.shType = dfTYPE_QUIT;
				header.shStrLen = 0;
			}
		}

		switch (header.shType)
		{
		case dfTYPE_ADD_STR:
			break;
		case dfTYPE_DEL_STR:
		case dfTYPE_PRINT_LIST:
			header.shStrLen = 0;
			break;
		case dfTYPE_QUIT:
			header.shStrLen = 0;
			loop = false;
			break;
		default:
			break;
		}
		//if (header.shStrLen == 9)
		//	system("pause");

		g_msgQ.Lock();
		if (g_msgQ.GetFreeSize() >= sizeof(header) + header.shStrLen * sizeof(WCHAR))
		{
			g_msgQ.Enqueue((char *)&header, sizeof(header));
			g_msgQ.Enqueue((char *)g_str.c_str(), header.shStrLen * sizeof(WCHAR));
			SetEvent(wakeEvent);
		}
		g_msgQ.UnLock();

		Sleep(DELAY_TIME);
	}
	DWORD state = WaitForMultipleObjects(WORKER_CNT, threadHandle, TRUE, INFINITE);

	printf("All Thread closed\n");
	system("pause");
	return 0;
}


unsigned __stdcall WorkerThread(LPVOID lpThreadParameter)
{
	st_MSG_HEAD header;
	std::wstring str;
	DWORD state;
	while (1)
	{
		printf("worker\n");
		state = WaitForSingleObject(wakeEvent,INFINITE);
		g_msgQ.Lock();
		if (g_msgQ.GetUseSize() < sizeof(header)) 
		{
			g_msgQ.UnLock();
			continue;
		}

		g_msgQ.Peek((char *)&header, sizeof(header));

		switch (header.shType)
		{
		case dfTYPE_ADD_STR:
			g_msgQ.Dequeue((char *)&header, sizeof(header));
			g_msgQ.Dequeue(str, header.shStrLen*sizeof(WCHAR));
			//str = str.c_str();
			addStr(str);
			g_msgQ.UnLock();
			
			break;
		case dfTYPE_DEL_STR:
			g_msgQ.Dequeue((char *)&header, sizeof(header));
			g_msgQ.UnLock();
			delStr();
			break;
		case dfTYPE_PRINT_LIST:
			g_msgQ.Dequeue((char *)&header, sizeof(header));
			g_msgQ.UnLock();
			printList();
			break;
		case dfTYPE_QUIT:
			g_msgQ.UnLock();
			SetEvent(wakeEvent);//종료될 다음 thread를 깨움
			//printf("worker quit");
			return 0;
			break;
		default:
			printf("fffff\n");
			g_msgQ.UnLock();
			break;
		}
		
	}
}

void addStr(std::wstring &str)
{
	AcquireSRWLockExclusive(&listLock);
	g_List.push_front(str);
	ReleaseSRWLockExclusive(&listLock);
}
void delStr()
{
	AcquireSRWLockExclusive(&listLock);
	if(!g_List.empty())
		g_List.pop_back();
	ReleaseSRWLockExclusive(&listLock);
}

void printList()
{
	AcquireSRWLockExclusive(&listLock);
	wprintf(L"List:");
	for (auto iter = g_List.begin(); iter != g_List.end(); iter++)
	{
		wprintf(L"[%s] ",(*iter).c_str());
	}
	wprintf(L"\n");
	ReleaseSRWLockExclusive(&listLock);

}