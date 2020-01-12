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
	
}