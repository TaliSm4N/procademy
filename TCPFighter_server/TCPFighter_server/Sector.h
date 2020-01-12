#ifndef __SECTOR__
#define __SECTOR__

#define dfSECTOR_LEN_X 320
#define dfSECTOR_LEN_Y 320

#define dfSECTOR_MAX_X (dfRANGE_MOVE_RIGHT/dfSECTOR_LEN_X)
#define dfSECTOR_MAX_Y (dfRANGE_MOVE_BOTTOM/dfSECTOR_LEN_Y)


struct SectorPos
{
	int x;
	int y;
};

struct SectorAround
{
	int count;
	SectorPos around[9];
};

extern std::list<Player *> g_Sector[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];

void SectorAddPlayer(Player *player);

#endif // !__SECTOR__
