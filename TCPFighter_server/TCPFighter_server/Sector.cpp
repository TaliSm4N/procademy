#include <list>
#include <Windows.h>
#include <map>

#include "Packet.h"
#include "RingBuffer.h"
#include "network.h"
#include "Game.h"
#include "Sector.h"

std::list<Player *> g_Sector[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];

void SectorAddPlayer(Player *player)
{
	player->curSector.x = player->x / dfSECTOR_LEN_X;
	player->curSector.y = player->y / dfSECTOR_LEN_Y;

	g_Sector[player->curSector.y][player->curSector.x].push_back(player);
}

void SectorRemovePlayer(Player *player)
{
	g_Sector[player->curSector.y][player->curSector.x].remove(player);
	player->oldSector.x = player->curSector.x;
	player->oldSector.y = player->curSector.y;
}	
bool SectorUpdateCharacter(Player *player)
{
	if (player->x / dfSECTOR_LEN_X == player->curSector.x&&player->y / dfSECTOR_LEN_Y == player->curSector.y)
		return false;

	SectorRemovePlayer(player);
	SectorAddPlayer(player);

	return true;
}

void GetSectorAround(int x, int y, SectorAround *sectorAround)
{
	//sectorAround->count = 9;
	sectorAround->count = 0;

	for (int i = 0; i < 3; i++)
	{
		if (x - 1 + i < 0 || x - 1 + i >= dfSECTOR_MAX_X)
			continue;

		for (int j = 0; j < 3; j++)
		{
			if (y - 1 + j < 0 || y - 1 + j >= dfSECTOR_MAX_X)
				continue;
			
			sectorAround->around[sectorAround->count].x = x - 1 + i;
			sectorAround->around[sectorAround->count].y = y - 1 + j;
			sectorAround->count++;
			
		}
	}
}

void GetSectorAround(SectorPos &pos, SectorAround *sectorAround)
{
	GetSectorAround(pos.x, pos.y, sectorAround);
}

void GetUpdateSectorAround(Player *player, SectorAround *removeSector, SectorAround *addSector)
{
	SectorAround oldSectorAround;
	SectorAround curSectorAround;

	bool find = false;

	oldSectorAround.count = 0;
	curSectorAround.count = 0;

	removeSector->count = 0;
	addSector->count = 0;

	GetSectorAround(player->oldSector, &oldSectorAround);
	GetSectorAround(player->curSector, &curSectorAround);

	for (int i = 0; i < oldSectorAround.count; i++)
	{
		find = false;
		for (int j = 0; j < curSectorAround.count; j++)
		{
			if (oldSectorAround.around[i] == curSectorAround.around[j])
			{
				find = true;
				break;
			}
		}
		if (!find)
		{
			removeSector->around[removeSector->count] = oldSectorAround.around[i];
			removeSector->count++;
		}
	}

	for (int i = 0; i < curSectorAround.count; i++)
	{
		find = false;
		for (int j = 0; j < oldSectorAround.count; j++)
		{
			if (curSectorAround.around[i] == oldSectorAround.around[j])
			{
				find = true;
				break;
			}
		}
		if (!find)
		{
			addSector->around[addSector->count] = curSectorAround.around[i];
			addSector->count++;
		}
	}
}