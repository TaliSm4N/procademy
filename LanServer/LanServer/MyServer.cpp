#include "LanServerLib.h"
#include "Player.h"
#include "MyServer.h"


CMyServer::CMyServer()
{
}

bool CMyServer::OnConnectionRequest(WCHAR *ClientIP, int Port)
{
	return true;
}

void CMyServer::OnRecv(DWORD sessionID, Packet *p)
{
	Echo(sessionID, p);
}

void CMyServer::OnSend(DWORD sessionID, int sendsize)
{
	return;
}
void CMyServer::OnClientJoin(DWORD sessionID)
{
	Player *player = new Player(sessionID);


	Packet *p = Packet::Alloc();

	*p << 0x7fffffffffffffff;



	AcquireSRWLockExclusive(&playerListLock);
	playerList.insert(std::make_pair(sessionID, player));
	SendPacket(sessionID, p);
	Packet::Free(p);



	ReleaseSRWLockExclusive(&playerListLock);
	
}
void CMyServer::OnClientLeave(DWORD sessionID)
{
	AcquireSRWLockExclusive(&playerListLock);
	auto iter = playerList.find(sessionID);

	if (iter == playerList.end())
	{
		ReleaseSRWLockExclusive(&playerListLock);
		return;
	}
	Player *player = iter->second;
	playerList.erase(playerList.find(sessionID));
	player->Lock();
	ReleaseSRWLockExclusive(&playerListLock);
	player->UnLock();
	delete player;
}
void CMyServer::OnError(int errorcode, WCHAR *)
{}


bool CMyServer::Echo(LONGLONG sessionID, Packet *p)
{
	//echo하기 전에 player에 삭제되면 echo할 이유가 없음
	AcquireSRWLockExclusive(&playerListLock);
	auto iter = playerList.find(sessionID);

	if (iter == playerList.end())
	{
		ReleaseSRWLockExclusive(&playerListLock);
		return FALSE;
	}
	Player *player = iter->second;
	ReleaseSRWLockExclusive(&playerListLock);


	SendPacket(sessionID, p);


	return TRUE;
}