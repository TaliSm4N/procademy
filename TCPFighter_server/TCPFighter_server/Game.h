#ifndef __GAME__
#define __GAME__

#define dfRANGE_MOVE_TOP	0
#define dfRANGE_MOVE_LEFT	0
#define dfRANGE_MOVE_RIGHT	6400
#define dfRANGE_MOVE_BOTTOM	6400

#define dfERROR_RANGE 20

#define MOVE_SPEED_X 3
#define MOVE_SPEED_Y 2

#define FRAME 50

#define dfSECTOR_LEN_X 150
#define dfSECTOR_LEN_Y 150

#define dfSECTOR_MAX_X (dfRANGE_MOVE_RIGHT/dfSECTOR_LEN_X + 1)
#define dfSECTOR_MAX_Y (dfRANGE_MOVE_BOTTOM/dfSECTOR_LEN_Y + 1)

#define dfATTACK_RANGE_X 80
#define dfATTACK_RANGE_Y 10

#define dfDAMAGE1 3
#define dfDAMAGE2 4
#define dfDAMAGE3 5

enum Status {
	STAND, MOVE, ATTACK1, ATTACK2, ATTACK3
};

struct SectorPos
{
	int x;
	int y;

	bool operator ==(SectorPos &s)
	{
		if (this->x == s.x&&this->y == s.y)
			return true;
		return false;
	}
};

struct SectorAround
{
	int count;
	SectorPos around[9];
};

struct Player
{
	Session *session;
	DWORD sessionID;
	BYTE direction;
	BYTE moveDirection;

	//ÁÂÇ¥
	WORD x;
	WORD y;

	BYTE HP;

	Status action;

	SectorPos curSector;
	SectorPos oldSector;
};

extern std::map<DWORD, Player *> g_playerMap;



Player *CreatePlayer(Session *session);
bool Update();
void Run();
bool MovePlayer(Player *player);
bool CheckPos(int x, int y);
void PlayerSectorUpdate(Player *player);

#endif // !__GAME__
