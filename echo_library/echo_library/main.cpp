#pragma comment(lib, "ws2_32")
#include <iostream>
#include <tchar.h>
#include <WinSock2.h>
#include <process.h>
#include <list>
#include "Packet.h"
#include "RingBuffer.h"
#include "Session.h"
#include "network.h"

int _tmain(int argc, _TCHAR* argv[])
{
	initNetwork();

	while (1)
	{
		system("pause");
	}
}