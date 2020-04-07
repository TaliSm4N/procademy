#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include <process.h>
#include <ctime>
#include <map>
#include "MemoryPool.h"
#include "MemoryPoolTLS.h"
#include "CrashDump.h"


unsigned int WINAPI MemoryPoolThread(LPVOID lpParam);
unsigned int WINAPI NewDeleteThread(LPVOID lpParam);


//최근 50000개만 기록한다
//맨앞 숫자(16진수) 1일때 alloc
//맨앞 숫자(16진수) 0일때 free
#define TRACK_MAX 100000

void *track[TRACK_MAX];
unsigned long long trackCur = 0;



struct st_TEST_DATA
{
	char data[112];
};


MemoryPool<st_TEST_DATA> memory;

//CrashDump *Dump;



///////////////////////////////////////////////////////////
//모니터링
///////////////////////////////////////////////////////////
int spinTotal = 0;
int popTotal = 0;
int pushTotal = 0;

int testCnt;
int testAmount;

int main()
{
	CrashDump();
	SYSTEM_INFO sysInfo;
	//timeBeginPeriod(1);
	GetSystemInfo(&sysInfo);

	//int threadCnt = 1;
	int threadCnt;

	DWORD id;

	printf("set Test Thread Count\n>> ");
	scanf("%d", &threadCnt);
	printf("set Test Amount\n>> ");
	scanf("%d", &testAmount);
	printf("set Test Count\n>> ");
	scanf("%d",&testCnt);


	for (int i = 0; i < threadCnt; i++)
	{
		_beginthreadex(NULL, 0, MemoryPoolThread, NULL, 0, (unsigned int *)&id);
		_beginthreadex(NULL, 0, NewDeleteThread, NULL, 0, (unsigned int *)&id);
	}

	while (1);
}


unsigned int WINAPI MemoryPoolThread(LPVOID lpParam)
{
	st_TEST_DATA *p;
	for (int i = 0; i < testCnt;i++)
	{
		for (int j = 0; j < testAmount; j++)
		{
			p = memory.Alloc();

			for (int index = 0; index < 100; index++)
			{
				p->data[index] = index;
			}

			memory.Free(p);
		}
	}

	return 0;
}

unsigned int WINAPI NewDeleteThread(LPVOID lpParam)
{
	st_TEST_DATA *p;
	for (int i = 0; i < testCnt; i++)
	{
		for (int j = 0; j < testAmount; j++)
		{
			p = new st_TEST_DATA;

			for (int index = 0; index < 100; index++)
			{
				p->data[index] = index;
			}

			delete p;
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