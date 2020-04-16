#include "LanServerLib.h"
#include "Player.h"


Player::Player(LONGLONG id)
	:sessionID(id)
{
	InitializeSRWLock(&playerLock);
}

void Player::Lock()
{
	AcquireSRWLockExclusive(&playerLock);
}

void Player::UnLock()
{
	ReleaseSRWLockExclusive(&playerLock);
}