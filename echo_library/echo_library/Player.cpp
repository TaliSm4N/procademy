#include <map>
#include <Windows.h>
#include "Packet.h"
#include "Player.h"

std::map < LONGLONG, Player *> playerList;
SRWLOCK playerListLock;

Player::Player(LONGLONG id)
	:sessionID(id)
{}