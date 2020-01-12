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

enum Status {
	STAND, MOVE, ATTACK1, ATTACK2, ATTACK3
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
};

extern std::map<DWORD, Player *> g_playerMap;



Player *CreatePlayer(Session *session);
void Update();
void Run();
bool MovePlayer(Player *player);
bool CheckPos(int x, int y);

#endif // !__GAME__
