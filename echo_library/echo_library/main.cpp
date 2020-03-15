#pragma comment(lib, "ws2_32")
#include <iostream>
#include <tchar.h>
#include <WinSock2.h>
#include <process.h>
#include <map>
#include "Packet.h"
#include "RingBuffer.h"
#include "Session.h"
#include "network.h"
#include "networkLib.h"

int _tmain(int argc, _TCHAR* argv[])
{
	initNetwork(); 
	InitContents();

	while (1)
	{
		//system("pause");
	}
}