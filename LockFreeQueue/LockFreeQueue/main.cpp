#include <iostream>
#include <Windows.h>
#include <process.h>
#include <ctime>
#include <map>
#include "LockFreeQueue.h"
#include "CrashDump.h"

unsigned int WINAPI WorkerThread(LPVOID lpParam);

struct st_TEST_DATA
{
	volatile LONG64 lData;
	volatile LONG64 lCount;
};


LockFreeQueue<st_TEST_DATA *> queue;

//CrashDump *Dump;



///////////////////////////////////////////////////////////
//모니터링
///////////////////////////////////////////////////////////
int spinTotal = 0;
int popTotal = 0;
int pushTotal = 0;

int main()
{
	CrashDump();
	SYSTEM_INFO sysInfo;
	//timeBeginPeriod(1);
	GetSystemInfo(&sysInfo);

	//int threadCnt = 1;
	//int threadCnt = sysInfo.dwNumberOfProcessors;
	int threadCnt = sysInfo.dwNumberOfProcessors*2;

	DWORD id;
	for (int i = 0; i < threadCnt; i++)
	{
		_beginthreadex(NULL, 0, WorkerThread, NULL, 0, (unsigned int *)&id);
	}

	int spinbefore = 0;
	int popBefore = 0;
	int pushBefore = 0;


	while (1)
	{
		printf("--------------------------------------------------\n");
		printf("thread Count     : %d\n", threadCnt);
		printf("test Use Count   : %d\n", queue.GetUseCount());
		printf("test Enq  TPS    : %d\n", pushTotal - pushBefore);
		printf("test Enq  Total  : %d\n", pushBefore = pushTotal);
		printf("test Deq  TPS    : %d\n", popTotal - popBefore);
		printf("test Deq  Total  : %d\n", popBefore = popTotal);
		printf("--------------------------------------------------\n");
		Sleep(1000);
	}
}


unsigned int WINAPI WorkerThread(LPVOID lpParam)
{

	srand(time(NULL));
	for (int i = 0; i < 3; i++)
	{
		st_TEST_DATA *temp = new st_TEST_DATA;
		temp->lCount = 0;
		temp->lData = 0x0000000055555555;

		if (!queue.Enqueue(temp))
		{
			CrashDump::Crash();
		}
		InterlockedIncrement((LONG *)&pushTotal);
		//stack.Push(temp);
	}
	Sleep(500);

	st_TEST_DATA *data[1000];

	while (1)
	{
		//200개는 꺼내기
		//int count = rand() % 300 + 701;
		int count = 3;

		for (int i = 0; i < count; i++)
		{
			//if (!stack.Pop(&data[i]))
			if(!queue.Dequeue(&data[i]))
			{
				CrashDump::Crash();
			}
			InterlockedIncrement((LONG *)&popTotal);

			if (data[i]->lData != 0x0000000055555555 || data[i]->lCount != 0)
			{
				CrashDump::Crash();
			}
		}

		for (int i = 0; i < count; i++)
		{
			InterlockedIncrement((LONG *)&data[i]->lData);
			InterlockedIncrement((LONG *)&data[i]->lCount);
		}

		for (int i = 0; i < count; i++)
		{
			if (data[i]->lData != 0x0000000055555556 || data[i]->lCount != 1)
			{
				CrashDump::Crash();
			}
		}

		for (int i = 0; i < count; i++)
		{
			InterlockedDecrement((LONG *)&data[i]->lData);
			InterlockedDecrement((LONG *)&data[i]->lCount);
		}

		Sleep(20);

		for (int i = 0; i < count; i++)
		{

			if (data[i]->lData != 0x0000000055555555 || data[i]->lCount != 0)
			{
				CrashDump::Crash();
			}
		}
		for (int i = 0; i < count; i++)
		{
			if(!queue.Enqueue(data[i]))
			{
				CrashDump::Crash();
			}
			InterlockedIncrement((LONG *)&pushTotal);
			//stack.Push(data[i]);

		}

	}

	return 0;
}
/*
// cmpxchg16b.c
// processor: x64
// compile with: /EHsc /O2
#include <stdio.h>
#include <intrin.h>

typedef struct _LARGE_INTEGER_128 {
	__int64 Int[2];
} LARGE_INTEGER_128, *PLARGE_INTEGER_128;

volatile LARGE_INTEGER_128 BigInt;

// This AtomicOp() function atomically performs:
//   BigInt.Int[1] += BigInt.Int[0]
//   BigInt.Int[0] += 1
void AtomicOp()
{
	LARGE_INTEGER_128 Comparand;
	Comparand.Int[0] = BigInt.Int[0];
	Comparand.Int[1] = BigInt.Int[1];
	do {
		; // nothing
	} while (_InterlockedCompareExchange128(BigInt.Int,
		Comparand.Int[0] + Comparand.Int[1],
		Comparand.Int[0] + 1,
		Comparand.Int) == 0);
}

// In a real application, several threads contend for the value
// of BigInt.
// Here we focus on the compare and exchange for simplicity.
int main(void)
{
	BigInt.Int[1] = 23;
	BigInt.Int[0] = 11;
	AtomicOp();
	printf("BigInt.Int[1] = %d, BigInt.Int[0] = %d\n",
		BigInt.Int[1], BigInt.Int[0]);
}
*/