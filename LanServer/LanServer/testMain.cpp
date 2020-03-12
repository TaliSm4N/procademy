#include <iostream>

#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <process.h>
#include <cstring>
#include <map>
#include "Packet.h"
#include "RingBuffer.h"
#include "Session.h"
#include "LanServer.h"
#include "Player.h"
#include "MyServer.h"

int main()
{
	CMyServer test;

	test.Start(6000, 3, true, 8000);

	while (1)
	{
		//system("cls");
		//wprintf(L"----------------------------------\n");
		//wprintf(L"sessionCount  : %8d\n",test.GetSessionCount());
		//wprintf(L"acceptTotal   : %8d\n",test.GetAcceptTotal());
		//wprintf(L"acceptTPS     : %8d\n",test.GettAcceptTPS());
		//wprintf(L"recvPacketTPS : %8d\n", test.GetRecvPacketTPS());
		//wprintf(L"sendPacketTPS : %8d\n", test.GetSendPacketTPS());
		//wprintf(L"----------------------------------\n");
		Sleep(1000);
	}

}