#ifndef __SECTOR__
#define __SECTOR__

extern std::list<Player *> g_Sector[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];

void SectorAddPlayer(Player *player);
void SectorRemovePlayer(Player *player);
bool SectorUpdateCharacter(Player *player);
void GetSectorAround(int x, int y, SectorAround *sectorAround);
void GetSectorAround(SectorPos &pos, SectorAround *sectorAround);
void GetUpdateSectorAround(Player *player, SectorAround *removeSector, SectorAround *addSector);

#endif // !__SECTOR__
