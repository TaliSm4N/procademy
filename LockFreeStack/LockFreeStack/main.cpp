#include <iostream>
#include <Windows.h>
#include <process.h>
#include <ctime>
#include "LockFreeStack.h"
#include "CrashDump.h"

unsigned int WINAPI WorkerThread(LPVOID lpParam);

struct st_TEST_DATA
{
	volatile LONG64 lData;
	volatile LONG64 lCount;
};


LockFreeStack<st_TEST_DATA *> stack;

CrashDump *Dump;

int main()
{
	Dump = new CrashDump();
	SYSTEM_INFO sysInfo;

	//timeBeginPeriod(1);
	GetSystemInfo(&sysInfo);
	srand(time(NULL));
	DWORD id;
	for (int i = 0; i < sysInfo.dwNumberOfProcessors*2; i++)
	{
		_beginthreadex(NULL, 0, WorkerThread, NULL, 0, (unsigned int *)&id);
	}

	while (1)
	{
		printf("--------------------------------------------------\n");
		printf("thread Count   : %d\n", sysInfo.dwNumberOfProcessors*2);
		printf("test Use Count : %d\n",stack.GetUseCount());
		printf("--------------------------------------------------\n");
		Sleep(1000);
	}
}


unsigned int WINAPI WorkerThread(LPVOID lpParam)
{
	

	for (int i = 0; i < 1000; i++)
	{
		st_TEST_DATA *temp = new st_TEST_DATA;
		temp->lCount = 0;
		temp->lData = 0x0000000055555555;

		stack.Push(temp);
	}
	Sleep(10);

	st_TEST_DATA *data[1000];

	while (1)
	{
		//200°³´Â ²¨³»±â
		int count = rand() % 800 + 201;

		for (int i = 0; i < count; i++)
		{
			stack.Pop(&data[i]);

			if (data[i]->lData != 0x0000000055555555)
			{
				Dump->Crash();
			}

			if (data[i]->lCount != 0)
			{
				Dump->Crash();
			}

			InterlockedIncrement((LONG *)&data[i]->lData);
			InterlockedIncrement((LONG *)&data[i]->lCount);
		}

		Sleep(5);

		for (int i = 0; i < count; i++)
		{
			if (data[i]->lData != 0x0000000055555556)
			{
				Dump->Crash();
			}

			if (data[i]->lCount != 1)
			{
				Dump->Crash();
			}

			InterlockedDecrement((LONG *)&data[i]->lData);
			InterlockedDecrement((LONG *)&data[i]->lCount);

			stack.Push(data[i]);
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