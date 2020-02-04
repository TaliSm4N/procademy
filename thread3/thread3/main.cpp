#include <iostream>
#include <Windows.h>
#include <process.h>
#include <list>
#include <ctime>
#include <conio.h>
#include <intrin.h>
#include "RingBuffer.h"


#define DELAY_TIME 0
#define WORKER_CNT 3

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
HANDLE exitEvent;
HANDLE threadHandle[WORKER_CNT];
DWORD threadID[WORKER_CNT];

int addCnt = 0;
int delCnt = 0;
int printCnt = 0;


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
	exitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
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
	
	//누락된 wakeEvent발생
	//worker thread가 적으면 wakeEvent가 유실될 때가 있음 
	while (1)
	{
		g_msgQ.Lock();//1
		if (g_msgQ.GetUseSize() > 0)
		{
			SetEvent(wakeEvent);
		}
		else
		{
			break;
		}
		g_msgQ.UnLock();//1
		Sleep(DELAY_TIME);
	}
	g_msgQ.UnLock();//1

	
	DWORD state = WaitForMultipleObjects(WORKER_CNT, threadHandle, TRUE, INFINITE);

 	printf("All Thread closed\n");
	printf("Add: %d Delete: %d Print: %d\n",addCnt,delCnt,printCnt);
	return 0;
}


unsigned __stdcall WorkerThread(LPVOID lpThreadParameter)
{
	st_MSG_HEAD header;
	std::wstring str;
	DWORD state;

	HANDLE events[2] = { wakeEvent,exitEvent };

	bool loop = true;

	while (loop)
	{
		state = WaitForMultipleObjects(2,events,FALSE, INFINITE);
		if (state == WAIT_OBJECT_0)
		{
			g_msgQ.Lock();//2

			if (g_msgQ.GetUseSize() >= sizeof(header))
			{
				g_msgQ.Dequeue((char *)&header, sizeof(header));
				switch (header.shType)
				{
				case dfTYPE_ADD_STR:
					g_msgQ.Dequeue(str, header.shStrLen * sizeof(WCHAR));
					//g_msgQ.UnLock();//2
					addStr(str);
					break;
				case dfTYPE_DEL_STR:
					//g_msgQ.UnLock();//2
					delStr();
					break;
				case dfTYPE_PRINT_LIST:
					//g_msgQ.UnLock();//2
					printList();
					break;
				case dfTYPE_QUIT:
					//g_msgQ.UnLock();//2
					loop = false;
					SetEvent(exitEvent);//다른 쓰레드들을 종료하는 이벤트
					break;
				default:
					//g_msgQ.UnLock();//2
					system("pause");
					break;
				}
			}
			g_msgQ.UnLock();//2
		}
		else if (state == WAIT_OBJECT_0+1)
		{
			loop = false;
			break;
		}
	}
	return 0;
}

void addStr(std::wstring &str)
{
	//AcquireSRWLockExclusive(&listLock);//a
	g_List.push_front(str);
	addCnt++;
	//ReleaseSRWLockExclusive(&listLock);//a
}
void delStr()
{
	//AcquireSRWLockExclusive(&listLock);//b
	if (!g_List.empty())
	{
		g_List.pop_back();
		delCnt++;
	}
	//ReleaseSRWLockExclusive(&listLock);//b
}

void printList()
{
	//AcquireSRWLockExclusive(&listLock);//c
	wprintf(L"List:");
	for (auto iter = g_List.begin(); iter != g_List.end(); iter++)
	{
		wprintf(L"[%s] ",(*iter).c_str());
	}
	wprintf(L"\n");
	printCnt++;
	//ReleaseSRWLockExclusive(&listLock);//c
}