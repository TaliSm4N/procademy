#include <iostream>

#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <process.h>
#include <cstring>
#include <map>
#include <stack>
#include <thread>
#include "MemoryPool.h"
#include "MemoryPoolTLS.h"
#include "LockFreeStack.h"
#include "Packet.h"
#include "PacketPtr.h"
#include "RingBuffer.h"
#include "Session.h"
#include "LanServer.h"
#include "Player.h"
#include "MyServer.h"
#include "CrashDump.h"

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
		wprintf(L"----------------------------------\n");
		Sleep(1000);
	}

}