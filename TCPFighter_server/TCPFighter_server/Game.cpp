#include <Windows.h>
#include <map>
#include <list>

#include "Packet.h"
#include "RingBuffer.h"
#include "Protocol.h"
#include "network.h"
#include "Game.h"

std::map<DWORD, Player *> g_playerMap;


Player *CreatePlayer(Session *session)
{
	//새 플레이어 세팅
	Player *player = new Player();
	player->session = session;
	player->sessionID = session->sessionID;
	player->x = rand() % dfRANGE_MOVE_RIGHT + dfRANGE_MOVE_LEFT;
	player->y = rand() % dfRANGE_MOVE_BOTTOM + dfRANGE_MOVE_TOP;
	player->direction = dfPACKET_MOVE_DIR_LL;
	player->moveDirection = dfPACKET_MOVE_DIR_LL;
	player->HP = 100;

	//플레이어 리스트에 등록
	g_playerMap.insert(std::make_pair(player->sessionID, player));

	return player;
}

void Update()
{
	static DWORD oldTick=timeGetTime();
	DWORD nowTick = timeGetTime();
	
	if (nowTick - oldTick >= 1000 / FRAME)
	{
		oldTick += 1000 / FRAME;
		Run();
	}
}

void Run()
{
	Player *player;
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