#include <iostream>

#include "LanClientLib.h"
#include "NetServerLib.h"
#include "Connector.h"
#include "ChatServer.h"


int main()
{
	//SYSTEM_INFO sysInfo;

	SYSLOG_LEVEL(LOG_DEBUG);

	CrashDump();

	//GetSystemInfo(&sysInfo);

	ChatServer test;

	//test.ConfigStart(L"ChatServer.cnf");

	test.Config(L"ChatServer.cnf");
	test.Start();

	while (1)
	{
		system("cls");
		wprintf(L"Chatting Server\n");
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
		wprintf(L"----------------------------------\n");
		wprintf(L"UpdateMsgPool  : %8d\n", test.GetUpdateMsgPoolCount());
		wprintf(L"UpdateMsgQ     : %8d\n", test.GetUpdateMsgQCount());
		wprintf(L"PlayerCount    : %8d\n", test.GetPlayerCount());
		wprintf(L"PlayerPool     : %8d\n", test.GetPlayerPoolCount());
		wprintf(L"----------------------------------\n");
		wprintf(L"SessionKey cnt : %8d\n", test.GetKeyCount());
		wprintf(L"SessionMiss    : %8d\n", test._sessionMiss);
		wprintf(L"SessionNotFound: %8d\n", test._sessionNotFound);
		wprintf(L"==================================\n\n");
		wprintf(L"Server Connector\n");
		wprintf(L"----------------------------------\n");
		wprintf(L"WoerkThread : %d\n", test.GetConnector().GetWorkerThreadCount());
		wprintf(L"----------------------------------\n");
		if (test.GetConnector().GetConnectFlag())
		{
		wprintf(L"Connect       : True\n");
		}
		else
		{
		wprintf(L"Connect       : False\n");
		}

		wprintf(L"recvPacketTPS : %8d\n", test.GetConnector()._recvPacketTPS);
		wprintf(L"sendPacketTPS : %8d\n", test.GetConnector()._sendPacketTPS);
		wprintf(L"----------------------------------\n");

		Sleep(1000);
	}
}