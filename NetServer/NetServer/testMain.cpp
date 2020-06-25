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

	//test.ConfigStart(L"ChatServer.cnf");

	test.Config(L"ChatServer.cnf", L"SERVER");
	test.Start();

	while (1)
	{
		system("cls");
		wprintf(L"----------------------------------\n");
		wprintf(L"WoerkThread : %d MaxUser : %d\n", test.GetWorkerThreadCount(), test.GetMaxUser());
		wprintf(L"----------------------------------\n");
		wprintf(L"sessionCount  : %8d\n", test.GetSessionCount());
		wprintf(L"acceptTotal   : %8d\n", test._acceptTotal);
		wprintf(L"acceptTPS     : %8d\n", test._acceptTPS);
		wprintf(L"acceptFail    : %8d\n", test._acceptFail);
		wprintf(L"conReqFail    : %8d\n", test._connectionRequestFail);
		wprintf(L"recvPacketTPS : %8d\n", test._recvPacketTPS);
		wprintf(L"sendPacketTPS : %8d\n", test._sendPacketTPS);
		wprintf(L"packetCount   : %8d\n", test._packetCount);
		wprintf(L"==================================\n");
		wprintf(L"UpdateMsgPool : %8d\n", test.GetUpdateMsgPoolCount());
		wprintf(L"UpdateMsgQ    : %8d\n", test.GetUpdateMsgQCount());
		wprintf(L"PlayerCount   : %8d\n", test.GetPlayerCount());
		wprintf(L"PlayerPool    : %8d\n", test.GetPlayerPoolCount());
		Sleep(1000);
	}
}