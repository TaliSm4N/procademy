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
#include "Sector.h"

bool ConnectSession(Session *session)
{
	Player *player = CreatePlayer(session);
	Packet p;
	std::list<Player *> *sectorList;
	_LOG(dfLOG_LEVEL_ALWAYS, L"# ConnectSession #\n SessionID %d: ID %d / Direction %d / X %d / Y %d / HP %d\n", session->sessionID,player->sessionID, player->direction, player->x, player->y, player->HP);
	

	MakePacketCreatePlayer(p, player->sessionID, player->direction, player->x, player->y, player->HP);

	SendUnicast(session, p);

	MakePacketCreateOtherPlayer(p, player->sessionID, player->direction, player->x, player->y, player->HP);

	SendBroadCastSectorAround(session, p);

	sectorList = &g_Sector[player->curSector.y][player->curSector.x];
	for (auto iter = sectorList->begin(); iter != sectorList->end(); iter++)
	{
		if ((*iter) == player)
			continue;

		MakePacketCreateOtherPlayer(p, (*iter)->sessionID, (*iter)->direction, (*iter)->x, (*iter)->y, (*iter)->HP);
		SendUnicast(player->session, p);

		//기존 player들에 대한 액션 메시지 전송
		switch ((*iter)->action)
		{
		case MOVE:
			MakePacketMoveStart(p, (*iter)->sessionID, (*iter)->moveDirection, (*iter)->x, (*iter)->y);
			SendUnicast(player->session, p);
			break;
		case ATTACK1:
			MakePacketAttack1(p, (*iter)->sessionID, (*iter)->direction, (*iter)->x, (*iter)->y);
			SendUnicast(session, p);
			break;
		case ATTACK2:
			MakePacketAttack2(p, (*iter)->sessionID, (*iter)->direction, (*iter)->x, (*iter)->y);
			SendUnicast(session, p);
			break;
		case ATTACK3:
			MakePacketAttack3(p, (*iter)->sessionID, (*iter)->direction, (*iter)->x, (*iter)->y);
			SendUnicast(session, p);
			break;
		}
	}

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
		_LOG(dfLOG_LEVEL_WARNING,L"# Sync Move Start #\nSessionID %d / ID %d / X %d / Y %d\n", session->sessionID, player->sessionID, player->x, player->y);
		//sync
		MakePacketSync(p, player->sessionID, player->x, player->y);

		//send sector
		SendBroadCastSectorAround(session, p,true);
		//SendUnicast(session, p);//test용 코드

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

	if (SectorUpdateCharacter(player))
	{
		PlayerSectorUpdate(player);
	}

	MakePacketMoveStart(p, player->sessionID, player->moveDirection, player->x, player->y);

	//session 보내기로 만들기
	SendBroadCastSectorAround(session, p);

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
		_LOG(dfLOG_LEVEL_WARNING, L"# Move Stop Sync #\nSessionID %d / ID %d / X %d / Y %d\n", session->sessionID, player->sessionID, player->x, player->y);
		//sync
		MakePacketSync(p, player->sessionID, player->x, player->y);


		//send sector
		//SendBroadCast(session, p,true);
		//SendUnicast(session, p);//test용 코드
		SendBroadCastSectorAround(session, p,true);

		x = player->x;
		y = player->y;
	}

	player->x = x;
	player->y = y;

	player->direction = direction;

	player->action = STAND;

	if (SectorUpdateCharacter(player))
	{
		PlayerSectorUpdate(player);
	}

	MakePacketMoveStop(p, player->sessionID, player->direction, player->x, player->y);

	SendBroadCastSectorAround(session, p);

	return true;
}

bool Attack1(Session *session, Packet &p)
{
	BYTE direction;
	WORD x;
	WORD y;
	Player *player;
	Player *hitPlayer=NULL;

	p >> direction >> x >> y;

	player = g_playerMap.find(session->sessionID)->second;

	if (player == NULL)
	{
		_LOG(dfLOG_LEVEL_ERROR, L"# Attack1 #\nSessionID %d Character Not Found\n", session->sessionID);
		return false;
	}

	_LOG(dfLOG_LEVEL_DEBUG, L"# Attack #\nSessionID %d / ID %d / Direction %d / X %d / Y %d\n", session->sessionID, player->sessionID, direction, x, y);

	player->x = x;
	player->y = y;

	player->direction = direction;

	player->action = ATTACK1;

	MakePacketAttack1(p, player->sessionID, player->direction, player->x, player->y);

	SendBroadCastSectorAround(session, p);

	hitPlayer=AttackCheck(player);

	if (hitPlayer != NULL)
	{
		if((int)hitPlayer->HP-dfDAMAGE1<=0)
			hitPlayer->HP = 0;
		else
			hitPlayer->HP -= dfDAMAGE1;

		MakePacketDamage(p, player->sessionID, hitPlayer->sessionID, hitPlayer->HP);
		SendBroadCastSectorAround(session, p, true);
	}

	return true;
}

bool Attack2(Session *session, Packet &p)
{
	BYTE direction;
	WORD x;
	WORD y;
	Player *player;
	Player *hitPlayer=NULL;

	p >> direction >> x >> y;

	player = g_playerMap.find(session->sessionID)->second;

	if (player == NULL)
	{
		_LOG(dfLOG_LEVEL_ERROR, L"# Attack2 #\nSessionID %d Character Not Found\n", session->sessionID);
		return false;
	}

	_LOG(dfLOG_LEVEL_DEBUG, L"# Attack2 #\nSessionID %d / ID %d / Direction %d / X %d / Y %d\n", session->sessionID, player->sessionID, direction, x, y);

	player->x = x;
	player->y = y;

	player->direction = direction;

	player->action = ATTACK2;

	MakePacketAttack2(p, player->sessionID, player->direction, player->x, player->y);

	SendBroadCastSectorAround(session, p);

	hitPlayer = AttackCheck(player);

	if (hitPlayer != NULL)
	{
		if ((int)hitPlayer->HP - dfDAMAGE1 <= 0)
			hitPlayer->HP = 0;
		else
			hitPlayer->HP -= dfDAMAGE1;

		MakePacketDamage(p, player->sessionID, hitPlayer->sessionID, hitPlayer->HP);
		SendBroadCastSectorAround(session, p, true);
	}

	return true;
}

bool Attack3(Session *session, Packet &p)
{
	BYTE direction;
	WORD x;
	WORD y;
	Player *player;
	Player *hitPlayer=NULL;

	p >> direction >> x >> y;

	player = g_playerMap.find(session->sessionID)->second;

	if (player == NULL)
	{
		_LOG(dfLOG_LEVEL_ERROR, L"# Attack3 #\nSessionID %d Character Not Found\n", session->sessionID);
		return false;
	}

	_LOG(dfLOG_LEVEL_DEBUG, L"# Attack3 #\nSessionID %d / ID %d / Direction %d / X %d / Y %d\n", session->sessionID, player->sessionID, direction, x, y);

	player->x = x;
	player->y = y;

	player->direction = direction;

	player->action = ATTACK3;

	MakePacketAttack3(p, player->sessionID, player->direction, player->x, player->y);

	SendBroadCastSectorAround(session, p);

	hitPlayer = AttackCheck(player);

	if (hitPlayer != NULL)
	{
		if ((int)hitPlayer->HP - dfDAMAGE1 <= 0)
			hitPlayer->HP = 0;
		else
			hitPlayer->HP -= dfDAMAGE1;

		MakePacketDamage(p, player->sessionID, hitPlayer->sessionID, hitPlayer->HP);
		SendBroadCastSectorAround(session, p,true);
	}

	return true;
}

Player *AttackCheck(Player *player)
{
	std::list<Player *> *listSector;// = &g_Sector[player->curSector.y][player->curSector.x];

	SectorAround sectorAround;

	GetSectorAround(player->curSector, &sectorAround);

	for (int i = 0; i < sectorAround.count; i++)
	{
		listSector = &g_Sector[sectorAround.around[i].y][sectorAround.around[i].x];

		for (auto iter = listSector->begin(); iter != listSector->end(); iter++)
		{
			if (player == *iter)
				continue;

			if (player->direction == dfPACKET_MOVE_DIR_LL && player->x > (*iter)->x&&player->x - (*iter)->x <= dfATTACK_RANGE_X)
			{
				if (abs(player->y - (*iter)->y) <= dfATTACK_RANGE_Y)
				{
					return *iter;
				}
			}
			else if (player->direction == dfPACKET_MOVE_DIR_RR && player->x < (*iter)->x &&(*iter)->x - player->x <= dfATTACK_RANGE_X)
			{
				if (abs(player->y - (*iter)->y) <= dfATTACK_RANGE_Y)
				{
					return *iter;
				}
			}

			
		}
	}

	return NULL;
}

bool Echo(Session *session, Packet &p)
{
	DWORD time;
	p >> time;

	MakePacketEcho(p, time);

	SendUnicast(session, p);

	return true;
}