#include <iostream>

#include "LanClientLib.h"
#include "MyClient.h"

int main()
{
	CrashDump();

	CMyClient test;
	test.initConnectInfo(L"127.0.0.1", 6000);
	if (!test.Start(3, true))
		return 0;

	while (1)
	{
	}
}