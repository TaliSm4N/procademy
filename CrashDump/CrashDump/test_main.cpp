#include <iostream>

#include "CrashDump.h"

int main()
{
	CrashDump CrashTest;

	printf("crash set\n");

	CrashTest.Crash();

	printf("crash\n");

	return 0;
}