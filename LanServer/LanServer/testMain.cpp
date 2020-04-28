#include <iostream>

#include "LanServerLib.h"
#include "Player.h"
#include "MyServer.h"


int main()
{
	SYSTEM_INFO sysInfo;

	CrashDump();
	
	GetSystemInfo(&sysInfo);

	CMyServer test;

	test.Start(6000, sysInfo.dwNumberOfProcessors, true, 8000);

	while (1)
	{
		system("cls");
		wprintf(L"----------------------------------\n");
		wprintf(L"WoerkThread : %d MaxUser : %d\n",test.GetWorkerThreadCount(),test.GetMaxUser());
		wprintf(L"----------------------------------\n");
		wprintf(L"sessionCount  : %8d\n",test.GetSessionCount());
		wprintf(L"acceptTotal   : %8d\n",test.GetAcceptTotal());
		wprintf(L"acceptTPS     : %8d\n",test.GettAcceptTPS());
		wprintf(L"acceptFail    : %8d\n", test.GetAcceptFail());
		wprintf(L"conReqFail    : %8d\n", test.GetConnectionRequestFail());
		wprintf(L"recvPacketTPS : %8d\n", test.GetRecvPacketTPS());
		wprintf(L"sendPacketTPS : %8d\n", test.GetSendPacketTPS());
		wprintf(L"packetCount   : %8d\n", test.GetPacketCount());
		wprintf(L"----------------------------------\n");
		wprintf(L"DisconCnt     : %8d\n", test._disconnectCount);
		wprintf(L"releaseCnt    : %8d\n", test._releaseCount);
		wprintf(L"recvOverlap   : %8d\n", test._recvOverlap);
		wprintf(L"sendOverlap   : %8d\n", test._sendOverlap);
		wprintf(L"sessionGet    : %8d\n", test._sessionGetCount);
		Sleep(1000);
	}

}