#include <Windows.h>
#include <map>
#include "Packet.h"
#include "RingBuffer.h"
#include "Session.h"
#include "network.h"
#include "define.h"
#include "PacketProc.h"
#include "Player.h"

bool Echo(LONGLONG sessionID, Packet &p)
{
	AcquireSRWLockExclusive(&playerListLock);
	Player *player=(*playerList.find(sessionID)).second;
	ReleaseSRWLockExclusive(&playerListLock);
	//Header header;
	//
	//header.len = 8;

	LONGLONG data;

	p >> data;

	p.Clear();
	//p.PutData((char *)&header, sizeof(header));
	p << data;

	SendPacket(sessionID, p);
	//SendUnicast(session, p);

	return TRUE;
}

