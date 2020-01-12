#pragma comment(lib, "winmm.lib")
#include <Windows.h>
#include <iostream>
#include <map>
#include <list>

#include "Packet.h"
#include "RingBuffer.h"
#include "network.h"
#include "Protocol.h"
#include "Log.h"
#include "Game.h"
#include "Sector.h"

int main()
{
	timeBeginPeriod(1);
	if (InitNetwork())
	{
		_LOG(dfLOG_LEVEL_DEBUG, L"listenSocket open success");
		//std::cout << "listenSocket이 정상적으로 열렸습니다" << std::endl;
	}
	else
	{
		_LOG(dfLOG_LEVEL_DEBUG, L"listenSocket failed");
		//std::cout << "listenSocket 생성 실패" << std::endl;
		return 1;
	}

	while (1)
	{
		NetworkProcess();

		Update();
	}

	return 0;
}