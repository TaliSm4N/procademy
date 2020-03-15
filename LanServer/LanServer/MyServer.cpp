#include <Windows.h>
#include <map>
#include "Packet.h"
#include "RingBuffer.h"
#include "Session.h"
#include "LanServer.h"
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
	//switch (type)
	//{
	//case 0:
		Echo(sessionID, p);
		//break;
	//}
}

void CMyServer::OnSend(DWORD sessionID, int sendsize)
{
	return;
}
void CMyServer::OnClientJoin(DWORD sessionID)
{
	LanServerHeader header;
	Player *player = new Player(sessionID);
	Packet *p = new Packet;
	header.len = 8;
	p->PutData((char *)&header, sizeof(header));
	*p << 0x7fffffffffffffff;
	AcquireSRWLockExclusive(&playerListLock);
	playerList.insert(std::make_pair(sessionID, player));
	SendPacket(sessionID, p);
	//¿ä³ð ¹®Á¦

	ReleaseSRWLockExclusive(&playerListLock);
	//printf("player join %lld\n", player->GetID());
	
}
void CMyServer::OnClientLeave(DWORD sessionID)
{
	AcquireSRWLockExclusive(&playerListLock);
	Player *player = (*playerList.find(sessionID)).second;
	playerList.erase(playerList.find(sessionID));
	player->Lock();
	ReleaseSRWLockExclusive(&playerListLock);
	//printf("player join %d\n", player->GetID());
	player->UnLock();
	delete player;
}
void CMyServer::OnError(int errorcode, WCHAR *)
{}


bool CMyServer::Echo(LONGLONG sessionID, Packet *p)
{
	Packet *sendPacket = new Packet;
	AcquireSRWLockExclusive(&playerListLock);
	Player *player = (*playerList.find(sessionID)).second;
	ReleaseSRWLockExclusive(&playerListLock);
	LanServerHeader header;
	//
	//header.len = 8;

	LONGLONG data;

	*p >> data;

	header.len = sizeof(data);
	sendPacket->PutData((char *)&header, sizeof(header));
	*sendPacket << data;

	SendPacket(sessionID, sendPacket);
	//SendUnicast(session, p);

	return TRUE;
}