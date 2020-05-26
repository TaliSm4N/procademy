#include <iostream>

#include "NetServerLib.h"
#include "ChatServer.h"


int main()
{
	//SYSTEM_INFO sysInfo;

	SYSLOG_LEVEL(LOG_DEBUG);

	CrashDump();

	//GetSystemInfo(&sysInfo);

	ChatServer test;

	test.ConfigStart(L"ChatServer.cnf");

	while (1)
	{
		system("cls");
		wprintf(L"----------------------------------\n");
		wprintf(L"WoerkThread : %d MaxUser : %d\n", test.GetWorkerThreadCount(), test.GetMaxUser());
		wprintf(L"----------------------------------\n");
		wprintf(L"sessionCount  : %8d\n", test.GetSessionCount());
		wprintf(L"acceptTotal   : %8d\n", test.GetAcceptTotal());
		wprintf(L"acceptTPS     : %8d\n", test.GettAcceptTPS());
		wprintf(L"acceptFail    : %8d\n", test.GetAcceptFail());
		wprintf(L"conReqFail    : %8d\n", test.GetConnectionRequestFail());
		wprintf(L"recvPacketTPS : %8d\n", test.GetRecvPacketTPS());
		wprintf(L"sendPacketTPS : %8d\n", test.GetSendPacketTPS());
		wprintf(L"packetCount   : %8d\n", test.GetPacketCount());
		wprintf(L"==================================\n");
		wprintf(L"UpdateMsgPool : %8d\n", test.GetUpdateMsgPoolCount());
		wprintf(L"UpdateMsgQ    : %8d\n", test.GetUpdateMsgQCount());
		wprintf(L"PlayerCount   : %8d\n", test.GetPlayerCount());
		wprintf(L"PlayerPool    : %8d\n", test.GetPlayerPoolCount());
		wprintf(L"----------------------------------\n");
		wprintf(L"DisconCnt     : %8d\n", test._disconnectCount);
		wprintf(L"releaseCnt    : %8d\n", test._releaseCount);
		wprintf(L"releaseClose  : %8d\n", test._releaseClose);
		wprintf(L"recvOverlap   : %8d\n", test._recvOverlap);
		wprintf(L"sendOverlap   : %8d\n", test._sendOverlap);
		wprintf(L"sessionGet    : %8d\n", test._sessionGetCount);
		Sleep(1000);
	}
}