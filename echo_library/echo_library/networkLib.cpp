#include <map>
#include <Windows.h>
#include <iostream>
#include "Packet.h"
#include "RingBuffer.h"
#include "Session.h"
#include "Player.h"
#include "PacketProc.h"
#include "define.h"
#include "thread.h"
#include "Player.h"
#include "networkLib.h"

//SRWLOCK playerListLock;

void OnRecv(LONGLONG sessionID,Packet &p,int type)
{
	//Session *session;
	//Header header;

	//session=(*sessionList.find(sessionID)).second;
	//printf("OnRecv\n");
	switch (type)
	{
	case 0:
		Echo(sessionID, p);
		break;
	}
}

void OnClientLeave(LONGLONG sessionID)
{
	Player *player = (*playerList.find(sessionID)).second;
	AcquireSRWLockExclusive(&playerListLock);
	playerList.erase(playerList.find(sessionID));
	ReleaseSRWLockExclusive(&playerListLock);
	printf("player join %d\n", player->GetID());
	delete player;
}

void OnClientJoin(LONGLONG sessionID)
{
	Player *player = new Player(sessionID);

	AcquireSRWLockExclusive(&playerListLock);
	playerList.insert(std::make_pair(sessionID, player));
	ReleaseSRWLockExclusive(&playerListLock);
	printf("player join %d\n",player->GetID());
}

void InitContents()
{
	InitializeSRWLock(&playerListLock);
}