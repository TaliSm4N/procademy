#include<Windows.h>
#include <map>
#include <list>

#include "Packet.h"
#include "RingBuffer.h"
#include "network.h"
#include "Protocol.h"
#include "Log.h"
#include "Game.h"
#include "MakePacket.h"
#include "PacketProc.h"

bool ConnectSession(Session *session)
{
	st_PACKET_HEADER header;
	Player *player = CreatePlayer(session);
	Packet p;
	_LOG(dfLOG_LEVEL_DEBUG, L"# ConnectSession #\n SessionID %d: ID %d / Direction %d / X %d / Y %d / HP %d\n", session->sessionID,player->sessionID, player->direction, player->x, player->y, player->HP);
	

	MakePacketCreatePlayer(session, p, player->sessionID, player->direction, player->x, player->y, player->HP);

	SendUnicast(session, p);

	return true;
}

bool MoveStart(Session *session, Packet &p)
{
	BYTE direction;
	WORD x;
	WORD y;
	Player *player;

	Packet sendPacket;
	p >> direction >> x >> y;

	player = g_playerMap.find(session->sessionID)->second;

	if (player == NULL)
	{
		_LOG(dfLOG_LEVEL_ERROR, L"# MoveStart #\nSessionID %d Character Not Found\n", session->sessionID);
		return false;
	}

	_LOG(dfLOG_LEVEL_DEBUG, L"# MoveStart #\nSessionID %d / ID %d / Direction %d / X %d / Y %d\n", session->sessionID, player->sessionID, direction, x, y);

	

	//위치차이가 심할 경우 서버를 기준으로 싱크맞춤
	if (abs(player->x - x) > dfERROR_RANGE || abs(player->y - y) > dfERROR_RANGE)
	{
		_LOG(dfLOG_LEVEL_DEBUG,L"# Sync #\nSessionID %d / ID %d / X %d / Y %d\n", session->sessionID, player->sessionID, player->x, player->y);
		//sync
		MakePacketSync(session, p, player->sessionID, player->x, player->y);

		//send sector
		//SendBroadCast(session, p,true);
		SendUnicast(session, p);//test용 코드

		x = player->x;
		y = player->y;
	}

	player->moveDirection = direction;

	switch (direction)
	{
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RD:
		player->direction = dfPACKET_MOVE_DIR_RR;
		break;
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LD:
		player->direction = dfPACKET_MOVE_DIR_LL;
		break;
	}

	player->x = x;
	player->y = y;

	player->action = MOVE;

	MakePacketMoveStart(session, p, player->sessionID, player->moveDirection, player->x, player->y);

	//session 보내기로 만들기
	//SendBroadCast(session, p);

	return true;
}

bool MoveStop(Session *session, Packet &p)
{
	BYTE direction;
	WORD x;
	WORD y;
	Player *player;
	p >> direction >> x >> y;
	player = g_playerMap.find(session->sessionID)->second;

	if (player == NULL)
	{
		_LOG(dfLOG_LEVEL_ERROR, L"# MoveStop #\nSessionID %d Character Not Found\n", session->sessionID);
		return false;
	}

	_LOG(dfLOG_LEVEL_DEBUG, L"# MoveStop #\nSessionID %d / ID %d / Direction %d / X %d / Y %d\n", session->sessionID, player->sessionID, direction, x, y);

	//위치차이가 심할 경우 서버를 기준으로 싱크맞춤
	if (abs(player->x - x) > dfERROR_RANGE || abs(player->y - y) > dfERROR_RANGE)
	{
		_LOG(dfLOG_LEVEL_DEBUG, L"# Sync #\nSessionID %d / ID %d / X %d / Y %d\n", session->sessionID, player->sessionID, player->x, player->y);
		//sync
		MakePacketSync(session, p, player->sessionID, player->x, player->y);

		//send sector
		//SendBroadCast(session, p,true);
		SendUnicast(session, p);//test용 코드

		x = player->x;
		y = player->y;
	}

	player->direction = direction;

	player->action = STAND;

	return true;
}