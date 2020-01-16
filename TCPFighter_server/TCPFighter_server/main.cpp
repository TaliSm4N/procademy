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
#include "monitor.h"

int main()
{
	timeBeginPeriod(1);

	if (InitNetwork())
	{
		_LOG(dfLOG_LEVEL_ALWAYS, L"listenSocket open success");
		//std::cout << "listenSocket이 정상적으로 열렸습니다" << std::endl;
	}
	else
	{
		_LOG(dfLOG_LEVEL_ERROR, L"listenSocket failed");
		//std::cout << "listenSocket 생성 실패" << std::endl;
		return 1;
	}

	monitorUnit.MonitorInit();

	while (1)
	{
		
		NetworkProcess();
		
		Update();
		

		ServerControl();

		monitorUnit.Run();
	}

	return 0;
}