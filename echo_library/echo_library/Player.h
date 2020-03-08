#pragma once
#include <map>


class Player
{
public:
	Player(LONGLONG id);
	LONGLONG GetID() { return sessionID; }
	void Lock();
	void UnLock();
private:
	LONGLONG sessionID;
	SRWLOCK playerLock;
};

extern std::map < LONGLONG, Player *> playerList;
extern SRWLOCK playerListLock;