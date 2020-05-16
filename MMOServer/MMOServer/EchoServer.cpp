#include "MMOLib.h"
#include "player.h"
#include "EchoServer.h"

EchoServer::EchoServer(int iMaxSession, bool monitoring)
	:CMMOServer(iMaxSession,monitoring)
{
	player = new Player[iMaxSession];

	for (int i = 0; i < iMaxSession; i++)
	{
		SetSessionArray(i, &player[i]);
	}
}

void EchoServer::OnAuth_Update(void)
{

	return;
}
void EchoServer::OnGame_Update(void)
{
	return;
}
void EchoServer::OnError(int iErrorCode, WCHAR *szError)
{
	return;
}
