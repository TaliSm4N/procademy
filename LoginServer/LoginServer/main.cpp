#include <WinSock2.h>
#include <unordered_map>

#include <strsafe.h>
#pragma comment(lib,"mysqlclient.lib")
#include "include/mysql.h"
#include "include/errmsg.h"

#include "CommonProtocol.h"
#include "DBConnect.h"
#include "LanServerLib.h"
#include "ServerConnector.h"
#include "NetServerLib.h"
#include "loginServer.h"


int main()
{
	LoginServer test;

	test.Start(30000, 10, true, 1000);

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

		wprintf(L"----------------------------------\n");
		wprintf(L"DisconCnt     : %8d\n", test._disconnectCount);
		wprintf(L"releaseCnt    : %8d\n", test._releaseCount);
		wprintf(L"releaseClose  : %8d\n", test._releaseClose);
		wprintf(L"recvOverlap   : %8d\n", test._recvOverlap);
		wprintf(L"sendOverlap   : %8d\n", test._sendOverlap);
		wprintf(L"sessionGet    : %8d\n", test._sessionGetCount);
		wprintf(L"----------------------------------\n");
		wprintf(L"LoginSuccessTPS: %8d\n", test.successCnt);
		wprintf(L"Login Wait     : %8d\n", test.GetLoginWait());
		//wprintf(L"Login AvgTime  : %8d\n", test.loginAll/test.successCnt);
		wprintf(L"Login MaxTime  : %8u\n", test.loginMax);
		wprintf(L"Login MinTime  : %8u\n", test.loginMin);
		test.successCnt = 0;
		Sleep(1000);
	}
}