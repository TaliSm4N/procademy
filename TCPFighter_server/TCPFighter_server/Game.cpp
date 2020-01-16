#include <Windows.h>
#include <map>
#include <list>

#include "Packet.h"
#include "RingBuffer.h"
#include "Protocol.h"
#include "network.h"
#include "MakePacket.h"
#include "Game.h"
#include "Sector.h"
#include "monitor.h"

std::map<DWORD, Player *> g_playerMap;


Player *CreatePlayer(Session *session)
{
	//새 플레이어 세팅
	Player *player = new Player();
	player->session = session;
	player->sessionID = session->sessionID;
	//player->x = rand() % 320;
	//player->y = rand() % 320;
	player->x = rand() % dfRANGE_MOVE_RIGHT + dfRANGE_MOVE_LEFT;
	player->y = rand() % dfRANGE_MOVE_BOTTOM + dfRANGE_MOVE_TOP;
	player->direction = dfPACKET_MOVE_DIR_LL;
	player->moveDirection = dfPACKET_MOVE_DIR_LL;
	player->HP = 100;
	player->action = STAND;

	//플레이어 리스트에 등록
	g_playerMap.insert(std::make_pair(player->sessionID, player));
	SectorAddPlayer(player);

	//oldSector가 없음
	player->oldSector.x = -10;
	player->oldSector.y = -10;

	return player;
}

bool Update()
{
	static DWORD oldTick=timeGetTime();
	DWORD nowTick = timeGetTime();
	
	if (nowTick - oldTick >= 1000 / FRAME)
	{
		oldTick += 1000 / FRAME;
		//MonitorUnit.MonitorGameLogic(START);
		Run();
		//MonitorUnit.MonitorGameLogic(END);
		
		return true;
	}

	return false;
}

void Run()
{
	Player *player;

	monitorUnit.MonitorGameLogic(START);

	for (auto iter = g_playerMap.begin(); iter != g_playerMap.end(); )
	{
		player = iter->second;
		iter++;
		if (player->HP <= 0)
		{
			//사망처리
			DisconnectSession(player->session);
			continue;
		}

		switch (player->action)
		{
		case STAND:
			break;
		case MOVE:
			MovePlayer(player);
			break;
		case ATTACK1:
			break;
		case ATTACK2:
			break;
		case ATTACK3:
			break;
		}
	}

	monitorUnit.MonitorGameLogic(END);
}

bool MovePlayer(Player *player)
{
	switch (player->moveDirection)
	{
	case dfPACKET_MOVE_DIR_LL: 
		if (CheckPos(player->x - MOVE_SPEED_X, player->y))
		{
			player->x -= MOVE_SPEED_X;
		}
		break;
	case dfPACKET_MOVE_DIR_LU: 
		if (CheckPos(player->x - MOVE_SPEED_X, player->y - MOVE_SPEED_Y))
		{
			player->x -= MOVE_SPEED_X;
			player->y -= MOVE_SPEED_Y;
		}
		break;
	case dfPACKET_MOVE_DIR_UU: 
		if (CheckPos(player->x, player->y - MOVE_SPEED_Y))
		{
			player->y -= MOVE_SPEED_Y;
		}
		break;
	case dfPACKET_MOVE_DIR_RU: 
		if (CheckPos(player->x + MOVE_SPEED_X, player->y - MOVE_SPEED_Y))
		{
			player->x += MOVE_SPEED_X;
			player->y -= MOVE_SPEED_Y;
		}
		break;
	case dfPACKET_MOVE_DIR_RR: 
		if (CheckPos(player->x + MOVE_SPEED_X, player->y))
		{
			player->x += MOVE_SPEED_X;
		}
		break;
	case dfPACKET_MOVE_DIR_RD: 
		if (CheckPos(player->x + MOVE_SPEED_X, player->y + MOVE_SPEED_Y))
		{
			player->x += MOVE_SPEED_X;
			player->y += MOVE_SPEED_Y;
		}
		break;
	case dfPACKET_MOVE_DIR_DD: 
		if (CheckPos(player->x, player->y + MOVE_SPEED_Y))
		{
			player->y += MOVE_SPEED_Y;
		}
		break;
	case dfPACKET_MOVE_DIR_LD: 
		if (CheckPos(player->x - MOVE_SPEED_X, player->y + MOVE_SPEED_Y))
		{
			player->x -= MOVE_SPEED_X;
			player->y += MOVE_SPEED_Y;
		}
		break;
	}

	if (SectorUpdateCharacter(player))
	{
		PlayerSectorUpdate(player);
	}

	return true;
}

bool CheckPos(int x, int y)
{
	if (x >= dfRANGE_MOVE_LEFT && x < dfRANGE_MOVE_RIGHT)
	{
		if (y >= dfRANGE_MOVE_TOP && y < dfRANGE_MOVE_BOTTOM)
			return true;
	}

	return false;
}

void PlayerSectorUpdate(Player *player)
{
	SectorAround removeSector;
	SectorAround addSector;

	Packet p;

	std::list<Player *> *sectorList;

	GetUpdateSectorAround(player, &removeSector, &addSector);

	MakePacketRemovePlayer(p, player->sessionID);

	//삭제된 주변 sector에 remove player를 보냄
	for (int i = 0; i < removeSector.count; i++)
	{
		SendBroadCastSector(removeSector.around[i].x, removeSector.around[i].y, p);
	}

	//이동하는 플레이어에게 삭제된 player에 대한 remove player 보냄
	for (int i = 0; i < removeSector.count; i++)
	{
		sectorList = &g_Sector[removeSector.around[i].y][removeSector.around[i].x];
		for (auto iter = sectorList->begin(); iter != sectorList->end(); iter++)
		{
			MakePacketRemovePlayer(p, (*iter)->sessionID);
			SendUnicast(player->session, p);
		}
	}

	MakePacketCreateOtherPlayer(p, player->sessionID, player->direction, player->x, player->y, player->HP);

	for (int i = 0; i < addSector.count; i++)
	{
		SendBroadCastSector(addSector.around[i].x, addSector.around[i].y, p);
	}

	MakePacketMoveStart(p, player->sessionID, player->moveDirection, player->x, player->y);

	for (int i = 0; i < addSector.count; i++)
	{
		SendBroadCastSector(addSector.around[i].x, addSector.around[i].y, p);
	}

	for (int i = 0; i < addSector.count; i++)
	{
		sectorList = &g_Sector[addSector.around[i].y][addSector.around[i].x];
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
				SendUnicast(player->session, p);
				break;
			case ATTACK2:
				MakePacketAttack2(p, (*iter)->sessionID, (*iter)->direction, (*iter)->x, (*iter)->y);
				SendUnicast(player->session, p);
				break;
			case ATTACK3:
				MakePacketAttack3(p, (*iter)->sessionID, (*iter)->direction, (*iter)->x, (*iter)->y);
				SendUnicast(player->session, p);
				break;
			}
		}
	}

}

