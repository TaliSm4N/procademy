#include <WinSock2.h>
#include <unordered_map>

#include <strsafe.h>
#include <list>
#pragma comment(lib,"mysqlclient.lib")
#include "include/mysql.h"
#include "include/errmsg.h"

#include "CommonProtocol.h"
#include "DBConnect.h"
#include "DBConnectTLS.h"
#include "LanServerLib.h"
#include "ServerConnector.h"
#include "NetServerLib.h"
#include "loginServer.h"


int main()
{
	CrashDump();

	LoginServer test;

	//test.Start(30000, 10, true, 1000);

	if (!test.Config(L"Login.cnf"))
	{
		volatile int test = 1;
	}
	test.Start();

	while (1)
	{
		system("cls");
		wprintf(L"LoginServer\n");
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
		wprintf(L"LoginSuccessTPS: %8d\n", test.successCntTPS);
		wprintf(L"Login Wait     : %8d\n", test.GetLoginWait());
		if(test.successCnt>0)
			wprintf(L"Login AvgTime  : %8d\n", test.loginAll/test.successCnt);
		else
			wprintf(L"Login AvgTime  : %8d\n", 0);
		wprintf(L"Login MaxTime  : %8u\n", test.loginMax);
		wprintf(L"Login MinTime  : %8u\n", test.loginMin);
		wprintf(L"==================================\n\n");
		wprintf(L"ServerConnector\n");
		wprintf(L"----------------------------------\n");
		wprintf(L"WoerkThread : %d MaxUser : %d\n", test.GetConnector().GetWorkerThreadCount(), test.GetConnector().GetMaxUser());
		wprintf(L"----------------------------------\n");
		wprintf(L"ServerCount   : %8d\n", test.GetConnector().GetSessionCount());
		wprintf(L"acceptTotal   : %8d\n", test.GetConnector()._acceptTotal);
		wprintf(L"acceptTPS     : %8d\n", test.GetConnector()._acceptTPS);
		wprintf(L"acceptFail    : %8d\n", test.GetConnector()._acceptFail);
		wprintf(L"conReqFail    : %8d\n", test.GetConnector()._connectionRequestFail);
		wprintf(L"recvPacketTPS : %8d\n", test.GetConnector()._recvPacketTPS);
		wprintf(L"sendPacketTPS : %8d\n", test.GetConnector()._sendPacketTPS);
		wprintf(L"downMsg       : %8d\n",test.downMsg);
		wprintf(L"----------------------------------\n");

		test.successCntTPS = 0;
		Sleep(1000);
	}
}