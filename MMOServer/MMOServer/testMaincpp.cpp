#include <conio.h>
#include "MMOLib.h"
#include "player.h"
#include "EchoServer.h"

void ServerControl();

int main()
{
	CrashDump();
	SYSLOG_LEVEL(LOG_DEBUG);
	ProfileInit();

	 EchoServer *test;

	 test = new EchoServer(20000);

	 test->Start(NULL, 40000, 3, true, 119, 50);

	 while (1)
	 {
		 system("cls");
		 wprintf(L"----------------------------------\n");
		 wprintf(L"P : Profile | A : Sum | R : Profile Reset\n");
		 wprintf(L"----------------------------------\n");
		 wprintf(L"WoerkThread : %d MaxUser : %d\n", test->GetWorkerThreadCount(), test->GetMaxUser());
		 wprintf(L"----------------------------------\n");
		 wprintf(L"sessionCount  : %8d\n", test->_Monitor_SessionAllMode);
		 wprintf(L"acceptTotal   : %8d\n", test->_Monitor_AcceptSocket);
		 wprintf(L"acceptTPS     : %8d\n", test->_Monitor_Counter_Accept);
		 wprintf(L"acceptFail    : %8d\n", test->_Monitor_AcceptFail);
		 
		 wprintf(L"recvPacketTPS : %8d\n", test->_Monitor_Counter_PacketProc);
		 wprintf(L"sendPacketTPS : %8d\n", test->_Monitor_Counter_PacketSend);
		 //wprintf(L"packetCount   : %8d\n", Packet::PacketUseCount());
		 wprintf(L"packetCount   : %8d\n", test->_Monitor_Counter_Packet);
		 wprintf(L"==================================\n");
		 wprintf(L"AuthThreadFPS : %8d\n",test->_Monitor_Counter_AuthUpdate);
		 wprintf(L"GameThreadFPS : %8d\n", test->_Monitor_Counter_GameUpdate);
		 wprintf(L"AuthCnt       : %8d\n", test->_Monitor_SessionAuthMode);
		 wprintf(L"GameCnt       : %8d\n", test->_Monitor_SessionGameMode);
		 wprintf(L"----------------------------------\n");

		 ServerControl();

		 Sleep(1000);
	 }
}

void ServerControl()
{
	WCHAR controlKey;
	if (_kbhit())
	{
		controlKey = _getwch();

		if (controlKey == L'p' || controlKey == L'P')
		{
			ProfileDataOutText(L"profile");
		}
		else if (controlKey == L'a' || controlKey == L'A')
		{
			ProfileDataSumOutText(L"profileSum");
		}
		else if (controlKey == L'r' || controlKey == L'R')
		{
			ProfileReset();
		}
	}
}