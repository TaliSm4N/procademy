#include "monitor.h"
#include <conio.h>
#include <Windows.h>
#include <iostream>
#include <map>

#include "Log.h"
#include "Packet.h"
#include "RingBuffer.h"
#include "network.h"
#include "Game.h"


Monitor monitorUnit;

Monitor::Monitor()
	:checkTime(1000),min_check(60),\
	frame_cnt(0),sec_cnt(0),bNetwork(true)
{

}

void Monitor::MonitorInit()
{
	frame_cnt = 0;
	sec_cnt = 0;

	loop_cnt = 0;

	logicTick = 0;
	logicTickMin = -1;
	logicTickMax = 0;

	networkTick = 0;
	networkTickMin = -1;
	networkTickMax = 0;
}

void ServerControl()
{
	static bool controlMode = false;

	if (_kbhit())
	{
		WCHAR controlKey = _getwch();
		if (controlKey == L'm' || controlKey == L'M')
		{
			wprintf(L"--------------------\n");
			wprintf(L"M - How to use\n");
			wprintf(L"L - Key Lock\n");
			wprintf(L"U - Key Unlock\n");
			wprintf(L"I - Get Mode Info\n");
			wprintf(L"N - Network Monitoring on/off\n");
			wprintf(L"--------------------\n");
		}
		else if (controlKey == L'u' || controlKey == L'U')
		{
			controlMode = true;
			wprintf(L"Control Mode: Press L - Key Lock\n");
		}
		else if (controlKey == L'l' || controlKey == L'L')
		{
			controlMode = true;
			wprintf(L"ControlLock: Press U - Key Unlock\n");
		}

		if (controlMode)
		{
			if (controlKey==L'i'|| controlKey == L'I')
			{
				wprintf(L"--------------------\n");
				wprintf(L"Network Monitoring ");
				if (monitorUnit.GetNetwork()) wprintf(L"on\n");
				else wprintf(L"off\n");
				wprintf(L"--------------------\n");
			}
			else if (controlKey == L'n' || controlKey == L'N')
			{
				monitorUnit.SetNetwork();
				if (monitorUnit.GetNetwork())
					wprintf(L"Network monitoring on\n");
				else
					wprintf(L"Network monitoring off\n");
			}
		}
	}
}

void Monitor::Run()
{
	static int tick=timeGetTime();

	loop_cnt++;


	if (timeGetTime() - tick >= checkTime)
	{
		tick += checkTime;
		sec_cnt++;

		if (sec_cnt >= min_check || frame_cnt != FRAME)
		{
			_LOG(dfLOG_LEVEL_ALWAYS, L"Monitoring");
			if (bNetwork)
			{
				ShowNetwork();
			}
			MonitorInit();
		}
		frame_cnt = 0;
	}
}

void Monitor::ShowNetwork()
{
	_LOG_NOTIME(dfLOG_LEVEL_ALWAYS, L"\
-------------------------------------------\n\
Loop %d/s frame %d connectUser %05d\n\
-------------------------------------------\n\
logic(avr) %03dms network(avr) %03dms\n\
logic(min) %03dms network(min) %03dms\n\
logic(max) %03dms network(max) %03dms\n\
-------------------------------------------\n",\
loop_cnt / sec_cnt, frame_cnt, g_playerMap.size(),\
logicTick/loop_cnt, networkTick/ loop_cnt,\
logicTickMin, networkTickMin,\
logicTickMax, networkTickMax);
}

void Monitor::MonitorNetwork(MONITORING timing)
{
	static int startTime;
	static int endTime;

	if (timing == START)
		startTime = timeGetTime();
	else if (timing == END)
	{
		endTime = timeGetTime();

		if (networkTickMin > endTime - startTime)
			networkTickMin = endTime - startTime;
		if (networkTickMax < endTime - startTime)
			networkTickMax = endTime - startTime;

		networkTick += endTime - startTime;
	}
}

void Monitor::MonitorGameLogic(MONITORING timing)
{
	static int startTime;
	static int endTime;

	
	if (timing == START)
		startTime = timeGetTime();

	if (timing == END)
	{
		endTime = timeGetTime();

		if (logicTickMin > endTime - startTime)
			logicTickMin = endTime - startTime;
		if (logicTickMax < endTime - startTime)
			logicTickMax = endTime - startTime;
		logicTick += endTime - startTime;
		
		frame_cnt++;
	}
}