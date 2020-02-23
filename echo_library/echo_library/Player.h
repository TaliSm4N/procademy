#pragma once
#include <map>


class Player
{
public:
	Player(LONGLONG id);
	LONGLONG GetID() { return sessionID; }
private:
	LONGLONG sessionID;

};

extern std::map < LONGLONG, Player *> playerList;
extern SRWLOCK playerListLock;