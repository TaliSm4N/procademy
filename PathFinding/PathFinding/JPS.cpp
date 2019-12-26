#include "JPS.h"
#include <math.h>
#define inMap

//int direction[8][2] = { {1,0} ,{0,1},{-1,0},{0,-1} ,{-1,1},{-1,-1},{1,-1},{1,1} };


enum Direction {NONE=0,DIR_LL,DIR_RR,DIR_DD,DIR_UU,DIR_LU,DIR_LD,DIR_RU,DIR_RD};
static int search = SEARCH;

JPS::JPS(int w,int h)
	:successFind(false)
{
	map = new Map(w, h);
}

JPS::~JPS()
{
	delete map;
}

int JPS::Find()
{
 	NODE *checkNode=nullptr;
	NODE *newNode = nullptr;

	if (successFind)
		return 1;

	if (closeList.empty())
	{
		checkNode = new NODE();
		checkNode->x = map->GetStart().x;
		checkNode->y = map->GetStart().y;
		checkNode->parent = nullptr;
		checkNode->G = 0;
		checkNode->H = manhattan(map->GetStart().x, map->GetStart().y);
		checkNode->F = checkNode->G + checkNode->H;
		checkNode->pDir = NONE;
		closeList.push_front(checkNode);
	}
	else if (!openList.empty())
	{
		checkNode = openList.front();
		closeList.push_front(checkNode);
		openList.pop_front();
		if (map->getTile(checkNode->x, checkNode->y) != START && map->getTile(checkNode->x, checkNode->y) != END)
			map->setTile(CLOSE,checkNode->x, checkNode->y);
		if (checkNode->x == map->GetEnd().x&&checkNode->y == map->GetEnd().y)
		{
			successFind = true;
			map->successFind = true;
			correctionPath();
			return 1;
		}
	}
	else
	{
		closeList.back();
	}

	if (map->getTile(checkNode->x,checkNode->y)==END)
	{
		return 1;
	}
	for (int i = 1; i <= 8; i++)
	{
		int cX;
		int cY;
		bool c;
		c = false;

		switch (checkNode->pDir)
		{
		case NONE:
			c = true;
			break;
		case DIR_LL:
			if (i == DIR_LL||i==DIR_LU||i==DIR_LD)
				c = true;
			break;
		case DIR_RR:
			if (i == DIR_RR || i == DIR_RU || i == DIR_RD)
				c = true;
			break;
		case DIR_DD:
			if (i == DIR_DD || i == DIR_RD || i == DIR_LD)
				c = true;
			break;
		case DIR_UU:
			if (i == DIR_UU || i == DIR_RU || i == DIR_LU)
				c = true;
			break;
		case DIR_LU:
			if (!(i == DIR_RR && i == DIR_DD && i == DIR_RD))
				c = true;
			break;
		case DIR_LD:
			if (!(i == DIR_RR && i == DIR_UU && i == DIR_RU))
				c = true;
			break;
		case DIR_RU:
			if (!(i == DIR_LL && i == DIR_DD && i == DIR_LD))
				c = true;
			break;
		case DIR_RD:
			if (!(i == DIR_LL && i == DIR_UU && i == DIR_LU))
				c = true;
			break;
		}

		if (!c)
			continue;

		if (i <= 4)
		{
			if (searchStraight(i, checkNode->x,checkNode->y, cX, cY))
			{
				newNode = new NODE();
				newNode->x = cX;
				newNode->y = cY;
				newNode->parent = checkNode;
				newNode->G = checkNode->G + (abs(checkNode->x-cX)+ abs(checkNode->y - cY));
				newNode->H = manhattan(cX, cY);
				newNode->F = newNode->G + newNode->H;
				newNode->pDir = i;
				map->setTile(OPEN, cX, cY);
				pushOpenList(newNode);
			}
		}
		else
		{
			if (searchDiagonal(i, checkNode, cX, cY))
			{
				newNode = new NODE();
				newNode->x = cX;
				newNode->y = cY;
				newNode->parent = checkNode;
				newNode->G = checkNode->G + 1.5*(abs(checkNode->x - cX));
				newNode->H = manhattan(cX, cY);
				newNode->F = newNode->G + newNode->H;
				newNode->pDir = i;
				map->setTile(OPEN, cX, cY);
				pushOpenList(newNode);
			}
		}

	}

	search++;
	//set direct
	//for (int i = 0; i < 4; i++)
	//{
	//	if (map->getTile(checkNode->x + direction[i][0], checkNode->y + direction[i][1])==MODE::ROAD)
	//	{
	//		int x;
	//		int y;
	//		//int distance = getDistance(checkNode->x + direction[i][0], checkNode->y + direction[i][1],i);
	//		searchStraight(i, checkNode,x,y);
	//	}
	//}
	
	return 0;
}

bool JPS::searchStraight(int dir, int nX, int nY, int &cX, int &cY)
{

	switch (dir)
	{
	case DIR_LL:
		while (1)
		{
			nX -= 1;

			//if (map->getTile(nX, nY) == WRONG || map->getTile(nX, nY) == WALL)
			

			if (map->getTile(nX, nY) == END)
			{
				cX = nX;
				cY = nY;
				return true;
			}

			if (!(map->getTile(nX, nY) == ROAD))
			{
				if (map->getTile(nX, nY) >= SEARCH)
					continue;
				else
					return false;
			}

			if (map->getTile(nX, nY - 1) == WALL)
			{
				if (map->getTile(nX - 1, nY - 1) == ROAD)
				{
					cX = nX;
					cY = nY;
					
					return true;
				}
			}

			if (map->getTile(nX, nY + 1) == WALL)
			{
				if (map->getTile(nX - 1, nY + 1) == ROAD)
				{
					cX = nX;
					cY = nY;

					return true;
				}
			}

			map->setTile(search, nX, nY);
		}

		break;
	case DIR_RR:
		while (1)
		{
			nX += 1;

			//if (map->getTile(nX, nY) == WRONG || map->getTile(nX, nY) == WALL)
			

			if (map->getTile(nX, nY) == END)
			{
				cX = nX;
				cY = nY;
				return true;
			}

			if (!(map->getTile(nX, nY) == ROAD))
			{
				if (map->getTile(nX, nY) >= SEARCH)
					continue;
				else
					return false;
			}

			if (map->getTile(nX, nY - 1) == WALL)
			{
				if (map->getTile(nX + 1, nY - 1) == ROAD)
				{
					cX = nX;
					cY = nY;

					return true;
				}
			}

			if (map->getTile(nX, nY + 1) == WALL)
			{
				if (map->getTile(nX + 1, nY + 1) == ROAD)
				{
					cX = nX;
					cY = nY;

					return true;
				}
			}

			map->setTile(search, nX, nY);
		}
		break;
	case DIR_UU:
		while (1)
		{
			nY -= 1;

			//if (map->getTile(nX, nY) == WRONG || map->getTile(nX, nY) == WALL)


			if (map->getTile(nX, nY) == END)
			{
				cX = nX;
				cY = nY;
				return true;
			}

			if (!(map->getTile(nX, nY) == ROAD))
			{
				if (map->getTile(nX, nY) >= SEARCH)
					continue;
				else
					return false;
			}

			if (map->getTile(nX - 1, nY) == WALL)
			{
				if (map->getTile(nX - 1, nY - 1) == ROAD)
				{
					cX = nX;
					cY = nY;

					return true;
				}
			}

			if (map->getTile(nX + 1, nY) == WALL)
			{
				if (map->getTile(nX + 1, nY - 1) == ROAD)
				{
					cX = nX;
					cY = nY;

					return true;
				}
			}

			map->setTile(search, nX, nY);
		}
		break;
	case DIR_DD:
		while (1)
		{
			nY += 1;

			//if (map->getTile(nX, nY) == WRONG || map->getTile(nX, nY) == WALL)


			if (map->getTile(nX, nY) == END)
			{
				cX = nX;
				cY = nY;
				return true;
			}

			if (!(map->getTile(nX, nY) == ROAD))
			{
				if (map->getTile(nX, nY) >= SEARCH)
					continue;
				else
					return false;
			}

			if (map->getTile(nX - 1, nY) == WALL)
			{
				if (map->getTile(nX - 1, nY + 1) == ROAD)
				{
					cX = nX;
					cY = nY;

					return true;
				}
			}

			if (map->getTile(nX + 1, nY) == WALL)
			{
				if (map->getTile(nX + 1, nY + 1) == ROAD)
				{
					cX = nX;
					cY = nY;

					return true;
				}
			}

			map->setTile(search, nX, nY);
		}
		break;
	}

	return false;
}

bool JPS::searchDiagonal(int dir, NODE *checkNode, int &cX, int &cY)
{
	
	int nX = checkNode->x;
	int nY = checkNode->y;

	NODE *newNode;
	NODE *curNode;

	switch (dir)
	{
	case DIR_LD:
		while (1)
		{
			nX -= 1;
			nY += 1;

			//if (map->getTile(nX, nY) == WRONG || map->getTile(nX, nY) == WALL)

			if (map->getTile(nX, nY) == END)
			{
				cX = nX;
				cY = nY;
				return true;
			}

			if (!(map->getTile(nX, nY) == ROAD))
			{
				if (map->getTile(nX, nY) >= SEARCH)
					continue;
				else
					return false;
			}

			if (map->getTile(nX, nY - 1) == WALL)
			{
				if (map->getTile(nX - 1, nY - 1) == ROAD)
				{
					cX = nX;
					cY = nY;

					return true;
				}
			}

			if (map->getTile(nX + 1, nY) == WALL)
			{
				if (map->getTile(nX + 1, nY + 1) == ROAD)
				{
					cX = nX;
					cY = nY;

					return true;
				}
			}

			if (searchStraight(DIR_LL, nX, nY, cX, cY))
			{
				//newNode = new NODE();
				//newNode->x = nX;
				//newNode->y = nY;
				//newNode->parent = checkNode;
				//newNode->G = checkNode->G + 1;
				//newNode->H = manhattan(nX, nY);
				//newNode->F = newNode->G + newNode->H;
				//newNode->pDir = DIR_LD;
				//map->setTile(OPEN, cX, cY);
				//pushOpenList(newNode);
				//curNode = newNode;
				//
				//newNode = new NODE();
				//newNode->x = cX;
				//newNode->y = cY;
				//newNode->parent = curNode;
				//newNode->G = curNode->G + 1;
				//newNode->H = manhattan(cX, cY);
				//newNode->F = newNode->G + newNode->H;
				//newNode->pDir = DIR_LL;
				//map->setTile(OPEN, cX, cY);
				//pushOpenList(newNode);
				//
				//if (map->getTile(cX, cY) == END)
				//	return true;
				//
				//return false;
				cX = nX;
				cY = nY;
				return true;
			}

			if (searchStraight(DIR_DD, nX, nY, cX, cY))
			{
				cX = nX;
				cY = nY;
				return true;
				//newNode = new NODE();
				//newNode->x = nX;
				//newNode->y = nY;
				//newNode->parent = checkNode;
				//newNode->G = checkNode->G + 1;
				//newNode->H = manhattan(nX, nY);
				//newNode->F = newNode->G + newNode->H;
				//newNode->pDir = DIR_LD;
				//map->setTile(OPEN, cX, cY);
				//pushOpenList(newNode);
				//curNode = newNode;
				//
				//newNode = new NODE();
				//newNode->x = cX;
				//newNode->y = cY;
				//newNode->parent = curNode;
				//newNode->G = curNode->G + 1;
				//newNode->H = manhattan(cX, cY);
				//newNode->F = newNode->G + newNode->H;
				//newNode->pDir = DIR_DD;
				//map->setTile(OPEN, cX, cY);
				//pushOpenList(newNode);
				//
				//if (map->getTile(cX, cY) == END)
				//	return true;
				//
				//return false;
			}

			map->setTile(search, nX, nY);
		}
		break;
	case DIR_LU:
		while (1)
		{
			nX -= 1;
			nY -= 1;

			//if (map->getTile(nX, nY) == WRONG || map->getTile(nX, nY) == WALL)


			if (map->getTile(nX, nY) == END)
			{
				cX = nX;
				cY = nY;
				return true;
			}

			if (!(map->getTile(nX, nY) == ROAD))
			{
				if (map->getTile(nX, nY) >= SEARCH)
					continue;
				else
					return false;
			}

			if (map->getTile(nX, nY + 1) == WALL)
			{
				if (map->getTile(nX - 1, nY + 1) == ROAD)
				{
					cX = nX;
					cY = nY;

					return true;
				}
			}

			if (map->getTile(nX + 1, nY) == WALL)
			{
				if (map->getTile(nX + 1, nY - 1) == ROAD)
				{
					cX = nX;
					cY = nY;

					return true;
				}
			}

			if (searchStraight(DIR_LL, nX, nY, cX, cY))
			{
				cX = nX;
				cY = nY;
				return true;
				//newNode = new NODE();
				//newNode->x = nX;
				//newNode->y = nY;
				//newNode->parent = checkNode;
				//newNode->G = checkNode->G + 1;
				//newNode->H = manhattan(nX, nY);
				//newNode->F = newNode->G + newNode->H;
				//newNode->pDir = DIR_LU;
				//map->setTile(OPEN, cX, cY);
				//pushOpenList(newNode);
				//curNode = newNode;
				//
				//newNode = new NODE();
				//newNode->x = cX;
				//newNode->y = cY;
				//newNode->parent = curNode;
				//newNode->G = curNode->G + 1;
				//newNode->H = manhattan(cX, cY);
				//newNode->F = newNode->G + newNode->H;
				//newNode->pDir = DIR_LL;
				//map->setTile(OPEN, cX, cY);
				//pushOpenList(newNode);
				//
				//if (map->getTile(cX, cY) == END)
				//	return true;
				//
				//return false;
			}

			if (searchStraight(DIR_UU, nX, nY, cX, cY))
			{
				cX = nX;
				cY = nY;
				return true;
				//newNode = new NODE();
				//newNode->x = nX;
				//newNode->y = nY;
				//newNode->parent = checkNode;
				//newNode->G = checkNode->G + 1;
				//newNode->H = manhattan(nX, nY);
				//newNode->F = newNode->G + newNode->H;
				//newNode->pDir = DIR_LU;
				//map->setTile(OPEN, cX, cY);
				//pushOpenList(newNode);
				//curNode = newNode;
				//
				//newNode = new NODE();
				//newNode->x = cX;
				//newNode->y = cY;
				//newNode->parent = curNode;
				//newNode->G = curNode->G + 1;
				//newNode->H = manhattan(cX, cY);
				//newNode->F = newNode->G + newNode->H;
				//newNode->pDir = DIR_UU;
				//map->setTile(OPEN, cX, cY);
				//pushOpenList(newNode);
				//
				//if (map->getTile(cX, cY) == END)
				//	return true;
				//
				//return false;
			}

			map->setTile(search, nX, nY);
		}
		break;
	case DIR_RD:
		while (1)
		{
			nX += 1;
			nY += 1;

			//if (map->getTile(nX, nY) == WRONG || map->getTile(nX, nY) == WALL)

			if (map->getTile(nX, nY) == END)
			{
				cX = nX;
				cY = nY;
				return true;
			}

			if (!(map->getTile(nX, nY) == ROAD))
			{
				if (map->getTile(nX, nY) >= SEARCH)
					continue;
				else
					return false;
			}

			if (map->getTile(nX, nY - 1) == WALL)
			{
				if (map->getTile(nX + 1, nY - 1) == ROAD)
				{
					cX = nX;
					cY = nY;

					return true;
				}
			}

			if (map->getTile(nX - 1, nY) == WALL)
			{
				if (map->getTile(nX - 1, nY + 1) == ROAD)
				{
					cX = nX;
					cY = nY;

					return true;
				}
			}

			if (searchStraight(DIR_RR, nX, nY, cX, cY))
			{
				cX = nX;
				cY = nY;
				return true;
				//newNode = new NODE();
				//newNode->x = nX;
				//newNode->y = nY;
				//newNode->parent = checkNode;
				//newNode->G = checkNode->G + 1;
				//newNode->H = manhattan(nX, nY);
				//newNode->F = newNode->G + newNode->H;
				//newNode->pDir = DIR_RD;
				//map->setTile(OPEN, cX, cY);
				//pushOpenList(newNode);
				//curNode = newNode;
				//
				//newNode = new NODE();
				//newNode->x = cX;
				//newNode->y = cY;
				//newNode->parent = curNode;
				//newNode->G = curNode->G + 1;
				//newNode->H = manhattan(cX, cY);
				//newNode->F = newNode->G + newNode->H;
				//newNode->pDir = DIR_RR;
				//map->setTile(OPEN, cX, cY);
				//pushOpenList(newNode);
				//
				//if (map->getTile(cX, cY) == END)
				//	return true;
				//
				//return false;
			}

			if (searchStraight(DIR_DD, nX, nY, cX, cY))
			{
				cX = nX;
				cY = nY;
				return true;
				//newNode = new NODE();
				//newNode->x = nX;
				//newNode->y = nY;
				//newNode->parent = checkNode;
				//newNode->G = checkNode->G + 1;
				//newNode->H = manhattan(nX, nY);
				//newNode->F = newNode->G + newNode->H;
				//newNode->pDir = DIR_RD;
				//map->setTile(OPEN, cX, cY);
				//pushOpenList(newNode);
				//curNode = newNode;
				//
				//newNode = new NODE();
				//newNode->x = cX;
				//newNode->y = cY;
				//newNode->parent = curNode;
				//newNode->G = curNode->G + 1;
				//newNode->H = manhattan(cX, cY);
				//newNode->F = newNode->G + newNode->H;
				//newNode->pDir = DIR_DD;
				//map->setTile(OPEN, cX, cY);
				//pushOpenList(newNode);
				//
				//if (map->getTile(cX, cY) == END)
				//	return true;
				//
				//return false;
			}

			map->setTile(search, nX, nY);
		}
		break;
	case DIR_RU:
		while (1)
		{
			nX += 1;
			nY -= 1;

			//if (map->getTile(nX, nY) == WRONG || map->getTile(nX, nY) == WALL)

			if (map->getTile(nX, nY) == END)
			{
				cX = nX;
				cY = nY;
				return true;
			}

			if (!(map->getTile(nX, nY) == ROAD))
			{
				if (map->getTile(nX, nY) >= SEARCH)
					continue;
				else
					return false;
			}

			if (map->getTile(nX, nY + 1) == WALL)
			{
				if (map->getTile(nX + 1, nY + 1) == ROAD)
				{
					cX = nX;
					cY = nY;

					return true;
				}
			}

			if (map->getTile(nX - 1, nY) == WALL)
			{
				if (map->getTile(nX - 1, nY - 1) == ROAD)
				{
					cX = nX;
					cY = nY;

					return true;
				}
			}

			if (searchStraight(DIR_RR, nX, nY, cX, cY))
			{
				cX = nX;
				cY = nY;
				return true;
				//newNode = new NODE();
				//newNode->x = nX;
				//newNode->y = nY;
				//newNode->parent = checkNode;
				//newNode->G = checkNode->G + 1;
				//newNode->H = manhattan(nX, nY);
				//newNode->F = newNode->G + newNode->H;
				//newNode->pDir = DIR_RU;
				//map->setTile(OPEN, cX, cY);
				//pushOpenList(newNode);
				//curNode = newNode;
				//
				//newNode = new NODE();
				//newNode->x = cX;
				//newNode->y = cY;
				//newNode->parent = curNode;
				//newNode->G = curNode->G + 1;
				//newNode->H = manhattan(cX, cY);
				//newNode->F = newNode->G + newNode->H;
				//newNode->pDir = DIR_RR;
				//map->setTile(OPEN, cX, cY);
				//pushOpenList(newNode);
				//
				//if (map->getTile(cX, cY) == END)
				//	return true;
				//
				//return false;
			}

			if (searchStraight(DIR_UU, nX, nY, cX, cY))
			{
				cX = nX;
				cY = nY;
				return true;
				//newNode = new NODE();
				//newNode->x = nX;
				//newNode->y = nY;
				//newNode->parent = checkNode;
				//newNode->G = checkNode->G + 1;
				//newNode->H = manhattan(nX, nY);
				//newNode->F = newNode->G + newNode->H;
				//newNode->pDir = DIR_RU;
				//map->setTile(OPEN, cX, cY);
				//pushOpenList(newNode);
				//curNode = newNode;
				//
				//newNode = new NODE();
				//newNode->x = cX;
				//newNode->y = cY;
				//newNode->parent = curNode;
				//newNode->G = curNode->G + 1;
				//newNode->H = manhattan(cX, cY);
				//newNode->F = newNode->G + newNode->H;
				//newNode->pDir = DIR_UU;
				//map->setTile(OPEN, cX, cY);
				//pushOpenList(newNode);
				//
				//if (map->getTile(cX, cY) == END)
				//	return true;
				//
				//return false;
			}

			map->setTile(search, nX, nY);
		}
		break;
	}

	return false;
}

/*
bool JPS::searchStraight(int dir, int nX,int nY,int &cX,int &cY)
{

	//int nX = checkNode->x;
	//int nY = checkNode->y;
	static int search = (int)MODE::SEARCH;
	bool wall[2] = { false,false };
	bool ret=false;

	if (map->getTile(nX, nY) == WALL || map->getTile(nX, nY) == WRONG)
		return false;

	switch (dir)
	{
	case DIR_LL:
		while (1)
		{
			nX -= 1;

			wall[0] = false;
			wall[1] = false;

			//if (map->getTile(nX, nY) == WALL || map->getTile(nX, nY) == WRONG)
			if (!(map->getTile(nX-1, nY) == ROAD||map->getTile(nX-1,nY)>=SEARCH))
				break;

			if (!(map->getTile(nX, nY) == START || map->getTile(nX, nY) == END))
				map->setTile(search, nX, nY);

			if (map->getTile(nX, nY + 1) == WALL)
			{
				wall[0] = true;
			}
			if (map->getTile(nX, nY - 1) == WALL)
			{
				wall[1] = true;
			}

			if (wall[0])
			{
				if (map->getTile(nX - 1, nY + 1) != WALL&& map->getTile(nX - 1, nY + 1) != WRONG)
				{
					cX = nX;
					cY = nY;
					ret = true;

					break;
				}
			}

			if (wall[1])
			{
				if (map->getTile(nX - 1, nY - 1) == WALL&&map->getTile(nX-1,nY-1)!=WRONG)
				{

					cX = nX;
					cY = nY;
					ret = true;

					break;

				}
			}
		}

		break;
	case DIR_RR:
		while (1)
		{
			nX += 1;

			wall[0] = false;
			wall[1] = false;

			//if (map->getTile(nX, nY) == WALL || map->getTile(nX, nY) == WRONG)
			if (!(map->getTile(nX+1, nY) == ROAD || map->getTile(nX+1, nY) >= SEARCH))
				break;

			if (!(map->getTile(nX, nY) == START || map->getTile(nX, nY) == END))
				map->setTile(search, nX, nY);

			if (map->getTile(nX, nY + 1) == WALL)
			{
				wall[0] = true;
			}
			if (map->getTile(nX, nY - 1) == ROAD)
			{
				//map->setTile(search, nX, nY - 1);
			}
			else if (map->getTile(nX, nY - 1) == WALL)
			{
				wall[1] = true;
			}

			if (wall[0])
			{
				if (map->getTile(nX + 1, nY + 1) != WALL&& map->getTile(nX + 1, nY + 1) != WRONG)
				{
					cX = nX;
					cY = nY;
					ret = true;
					break;

				}
			}

			if (wall[1])
			{
				if (map->getTile(nX + 1, nY - 1) != WALL&& map->getTile(nX + 1, nY - 1) != WRONG)
				{
					cX = nX;
					cY = nY;
					ret = true;
					break;

				}
			}
		}

		break;
	case DIR_DD:
		while (1)
		{
			nY += 1;

			wall[0] = false;
			wall[1] = false;

			//if (map->getTile(nX, nY) == WALL || map->getTile(nX, nY) == WRONG)
			if (!(map->getTile(nX, nY+1) == ROAD || map->getTile(nX, nY+1) >= SEARCH))
				break;

			if (!(map->getTile(nX, nY) == START || map->getTile(nX, nY) == END))
				map->setTile(search, nX, nY);

			if (map->getTile(nX+1, nY) == WALL)
			{
				wall[0] = true;
			}
			if (map->getTile(nX-1, nY) == WALL)
			{
				wall[1] = true;
			}

			if (wall[0])
			{
				if (map->getTile(nX + 1, nY + 1) != WALL&& map->getTile(nX + 1, nY + 1) != WRONG)
				{
					cX = nX;
					cY = nY;
					ret = true;
					break;
					
				}
			}

			if (wall[1])
			{
				if (map->getTile(nX - 1, nY + 1) != WALL&& map->getTile(nX - 1, nY + 1) != WRONG)
				{
					cX = nX;
					cY = nY;
					ret = true;
					break;
				}
			}

		
		}
		break;
	case DIR_UU:
		while (1)
		{
			nY -= 1;

			wall[0] = false;
			wall[1] = false;

			//if (map->getTile(nX, nY) == WALL || map->getTile(nX, nY) == WRONG)
			if (!(map->getTile(nX, nY-1) == ROAD || map->getTile(nX, nY-1) >= SEARCH))
				break;

			if (!(map->getTile(nX, nY) == START || map->getTile(nX, nY) == END))
				map->setTile(search, nX, nY);
			

			if (map->getTile(nX + 1, nY) == WALL)
			{
				wall[0] = true;
			}
			if (map->getTile(nX - 1, nY) == WALL)
			{
				wall[1] = true;
			}

			if (wall[0])
			{
				if (map->getTile(nX + 1, nY - 1) != WALL&& map->getTile(nX + 1, nY - 1) != WRONG)
				{
					
					cX = nX;
					cY = nY;
					ret = true;
					break;
					
				}
			}

			if (wall[1])
			{
				if (map->getTile(nX - 1, nY - 1) == WALL&& map->getTile(nX - 1, nY - 1) == WRONG)
				{
					
					cX = nX;
					cY = nY;
					ret = true;
					break;
					
				}
			}

			
		}
		break;
	default:
		return false;
	}

	return ret;


}

bool JPS::searchDiagonal(int dir, NODE *checkNode, int &cX, int &cY)
{
	int nX = checkNode->x;
	int nY = checkNode->y;
	static int search = (int)MODE::SEARCH;
	bool wall[2] = { false,false };
	NODE* newNode;

	//if (!(map->getTile(nX, nY) == ROAD || map->getTile(nX, nY) >= SEARCH))
	//	return false;

	switch (dir)
	{
	case DIR_LD:
		while (1)
		{
			nX -= 1;
			nY += 1;

			wall[0] = false;
			wall[1] = false;

			//if (map->getTile(nX, nY) == WALL || map->getTile(nX, nY) == WRONG)
			if (!(map->getTile(nX-1, nY+1) == ROAD || map->getTile(nX-1, nY+1) >= SEARCH))
				break;

			if (!(map->getTile(nX, nY) == START || map->getTile(nX, nY) == END))
				map->setTile(search, nX, nY);

			if (map->getTile(nX + 1, nY) == WALL)
			{
				wall[0] = true;
			}

			if (map->getTile(nX, nY - 1) == WALL)
			{
				wall[1] = true;
			}

			if (wall[0])
			{
				if (map->getTile(nX + 1, nY + 1) != WALL&& map->getTile(nX + 1, nY + 1) != WRONG)
				{
					newNode = new NODE();
					newNode->x = nX;
					newNode->y = nY;
					newNode->parent = checkNode;
					newNode->G = checkNode->G + 1;
					newNode->H = manhattan(nX, nY);
					newNode->F = newNode->G + newNode->H;
					newNode->pDir = dir;
					map->setTile(OPEN, nX, nY);
					checkNode = newNode;
					pushOpenList(newNode);
					break;
				}
			}

			if (wall[1])
			{
				if (map->getTile(nX - 1, nY - 1) == WALL&& map->getTile(nX - 1, nY - 1) == WRONG)
				{
					newNode = new NODE();
					newNode->x = nX;
					newNode->y = nY;
					newNode->parent = checkNode;
					newNode->G = checkNode->G + 1;
					newNode->H = manhattan(nX, nY);
					newNode->F = newNode->G + newNode->H;
					newNode->pDir = dir;
					map->setTile(OPEN, nX, nY);
					checkNode = newNode;
					pushOpenList(newNode);
					break;
				}
			}
			if (searchStraight(DIR_LL, nX,nY, cX, cY))
			{
				newNode = new NODE();
				newNode->x = nX;
				newNode->y = nY;
				newNode->parent = checkNode;
				newNode->G = checkNode->G + 1;
				newNode->H = manhattan(nX, nY);
				newNode->F = newNode->G + newNode->H;
				newNode->pDir = dir;
				map->setTile(OPEN, nX, nY);
				checkNode = newNode;
				pushOpenList(newNode);
				break;
			}
			if (searchStraight(DIR_DD, nX, nY, cX, cY))
			{
				newNode = new NODE();
				newNode->x = nX;
				newNode->y = nY;
				newNode->parent = checkNode;
				newNode->G = checkNode->G + 1;
				newNode->H = manhattan(nX, nY);
				newNode->F = newNode->G + newNode->H;
				newNode->pDir = dir;
				map->setTile(OPEN, nX, nY);
				checkNode = newNode;
				pushOpenList(newNode);
				break;
			}
		}
		break;
	case DIR_LU:
		while (1)
		{
			nX -= 1;
			nY -= 1;

			wall[0] = false;
			wall[1] = false;

			//if (map->getTile(nX, nY) == WALL || map->getTile(nX, nY) == WRONG)
			if (!(map->getTile(nX - 1, nY - 1) == ROAD || map->getTile(nX - 1, nY - 1) >= SEARCH))
				break;

			if (!(map->getTile(nX, nY) == START || map->getTile(nX, nY) == END))
				map->setTile(search, nX, nY);

			if (map->getTile(nX + 1, nY) == WALL)
			{
				wall[0] = true;
			}

			if (map->getTile(nX, nY + 1) == WALL)
			{
				wall[1] = true;
			}

			if (wall[0])
			{
				if (map->getTile(nX + 1, nY - 1) != WALL&& map->getTile(nX + 1, nY - 1) != WRONG)
				{
					newNode = new NODE();
					newNode->x = nX;
					newNode->y = nY;
					newNode->parent = checkNode;
					newNode->G = checkNode->G + 1;
					newNode->H = manhattan(nX, nY);
					newNode->F = newNode->G + newNode->H;
					newNode->pDir = dir;
					map->setTile(OPEN, nX, nY);
					checkNode = newNode;
					pushOpenList(newNode);
					break;
				}
			}

			if (wall[1])
			{
				if (map->getTile(nX - 1, nY - 1) != WALL&& map->getTile(nX - 1, nY - 1) != WRONG)
				{
					newNode = new NODE();
					newNode->x = nX;
					newNode->y = nY;
					newNode->parent = checkNode;
					newNode->G = checkNode->G + 1;
					newNode->H = manhattan(nX, nY);
					newNode->F = newNode->G + newNode->H;
					newNode->pDir = dir;
					map->setTile(OPEN, nX, nY);
					checkNode = newNode;
					pushOpenList(newNode);
					break;
				}
			}

			if (searchStraight(DIR_LL, nX, nY, cX, cY))
			{
				newNode = new NODE();
				newNode->x = nX;
				newNode->y = nY;
				newNode->parent = checkNode;
				newNode->G = checkNode->G + 1;
				newNode->H = manhattan(nX, nY);
				newNode->F = newNode->G + newNode->H;
				newNode->pDir = dir;
				map->setTile(OPEN, nX, nY);
				checkNode = newNode;
				pushOpenList(newNode);
				break;
			}
			if (searchStraight(DIR_UU, nX, nY, cX, cY))
			{
				newNode = new NODE();
				newNode->x = nX;
				newNode->y = nY;
				newNode->parent = checkNode;
				newNode->G = checkNode->G + 1;
				newNode->H = manhattan(nX, nY);
				newNode->F = newNode->G + newNode->H;
				newNode->pDir = dir;
				map->setTile(OPEN, nX, nY);
				checkNode = newNode;
				pushOpenList(newNode);
				break;
			}
		}
		break;
	case DIR_RD:
		while (1)
		{
			nX += 1;
			nY += 1;

			wall[0] = false;
			wall[1] = false;

			//if (map->getTile(nX, nY) == WALL || map->getTile(nX, nY) == WRONG)
			if (!(map->getTile(nX + 1, nY + 1) == ROAD || map->getTile(nX + 1, nY + 1) >= SEARCH))
				break;

			if (!(map->getTile(nX, nY) == START || map->getTile(nX, nY) == END))
				map->setTile(search, nX, nY);

			if (map->getTile(nX - 1, nY) == WALL)
			{
				wall[0] = true;
			}

			if (map->getTile(nX, nY - 1) == WALL)
			{
				wall[1] = true;
			}

			if (wall[0])
			{
				if (map->getTile(nX - 1, nY + 1) != WALL&& map->getTile(nX - 1, nY + 1) != WRONG)
				{
					newNode = new NODE();
					newNode->x = nX;
					newNode->y = nY;
					newNode->parent = checkNode;
					newNode->G = checkNode->G + 1;
					newNode->H = manhattan(nX, nY);
					newNode->F = newNode->G + newNode->H;
					newNode->pDir = dir;
					map->setTile(OPEN, nX, nY);
					checkNode = newNode;
					pushOpenList(newNode);
					break;
				}
			}

			if (wall[1])
			{
				if (map->getTile(nX + 1, nY - 1) != WALL&& map->getTile(nX + 1, nY - 1) != WRONG)
				{
					newNode = new NODE();
					newNode->x = nX;
					newNode->y = nY;
					newNode->parent = checkNode;
					newNode->G = checkNode->G + 1;
					newNode->H = manhattan(nX, nY);
					newNode->F = newNode->G + newNode->H;
					newNode->pDir = dir;
					map->setTile(OPEN, nX, nY);
					checkNode = newNode;
					pushOpenList(newNode);
					break;
				}
			}

			if (searchStraight(DIR_RR, nX, nY, cX, cY))
			{
				newNode = new NODE();
				newNode->x = nX;
				newNode->y = nY;
				newNode->parent = checkNode;
				newNode->G = checkNode->G + 1;
				newNode->H = manhattan(nX, nY);
				newNode->F = newNode->G + newNode->H;
				newNode->pDir = dir;
				map->setTile(OPEN, nX, nY);
				checkNode = newNode;
				pushOpenList(newNode);
				break;
			}
			if (searchStraight(DIR_DD, nX, nY, cX, cY))
			{
				newNode = new NODE();
				newNode->x = nX;
				newNode->y = nY;
				newNode->parent = checkNode;
				newNode->G = checkNode->G + 1;
				newNode->H = manhattan(nX, nY);
				newNode->F = newNode->G + newNode->H;
				newNode->pDir = dir;
				map->setTile(OPEN, nX, nY);
				checkNode = newNode;
				pushOpenList(newNode);
				break;
			}
		}
		break;
	case DIR_RU:
		while (1)
		{
			nX += 1;
			nY -= 1;

			wall[0] = false;
			wall[1] = false;

			//if (map->getTile(nX, nY) == WALL || map->getTile(nX, nY) == WRONG)
			if (!(map->getTile(nX + 1, nY - 1) == ROAD || map->getTile(nX + 1, nY - 1) >= SEARCH))
				break;

			if (!(map->getTile(nX, nY) == START || map->getTile(nX, nY) == END))
				map->setTile(search, nX, nY);

			if (map->getTile(nX - 1, nY) == WALL)
			{
				wall[0] = true;
			}

			if (map->getTile(nX, nY + 1) == WALL)
			{
				wall[1] = true;
			}

			if (wall[0])
			{
				if (map->getTile(nX - 1, nY - 1) != WALL&& map->getTile(nX - 1, nY - 1) != WRONG)
				{
					newNode = new NODE();
					newNode->x = nX;
					newNode->y = nY;
					newNode->parent = checkNode;
					newNode->G = checkNode->G + 1;
					newNode->H = manhattan(nX, nY);
					newNode->F = newNode->G + newNode->H;
					newNode->pDir = dir;
					map->setTile(OPEN, nX, nY);
					checkNode = newNode;
					pushOpenList(newNode);
					break;
				}
			}

			if (wall[1])
			{
				if (map->getTile(nX + 1, nY - 1) == ROAD)
				{
					newNode = new NODE();
					newNode->x = nX;
					newNode->y = nY;
					newNode->parent = checkNode;
					newNode->G = checkNode->G + 1;
					newNode->H = manhattan(nX, nY);
					newNode->F = newNode->G + newNode->H;
					newNode->pDir = dir;
					map->setTile(OPEN, nX, nY);
					checkNode = newNode;
					pushOpenList(newNode);
					break;
				}
			}

			if (searchStraight(DIR_RR, nX, nY, cX, cY))
			{
				newNode = new NODE();
				newNode->x = nX;
				newNode->y = nY;
				newNode->parent = checkNode;
				newNode->G = checkNode->G + 1;
				newNode->H = manhattan(nX, nY);
				newNode->F = newNode->G + newNode->H;
				newNode->pDir = dir;
				map->setTile(OPEN, nX, nY);
				checkNode = newNode;
				pushOpenList(newNode);
				break;
			}
			if (searchStraight(DIR_UU, nX, nY, cX, cY))
			{
				newNode = new NODE();
				newNode->x = nX;
				newNode->y = nY;
				newNode->parent = checkNode;
				newNode->G = checkNode->G + 1;
				newNode->H = manhattan(nX, nY);
				newNode->F = newNode->G + newNode->H;
				newNode->pDir = dir;
				map->setTile(OPEN, nX, nY);
				checkNode = newNode;
				pushOpenList(newNode);
				break;
			}
		}
		break;
	}

	return true;
	/*
	switch (dir)
	{
	case DIR_LD:

		while (1)
		{
			if (map->getTile(nX, nY + 1) == ROAD)
			{
				map->setTile(search, nX, nY + 1);
			}
			else if (map->getTile(nX, nY + 1) == WALL)
			{
				wall[0] = true;
			}

			if (map->getTile(nX - 1, nY) == ROAD)
			{
				map->setTile(search, nX - 1, nY);
			}
			else if (map->getTile(nX - 1, nY) == WALL)
			{
				wall[1] = true;
			}

			if (wall[0])
			{
				if (map->getTile(nX, nY + 2) == ROAD)
				{
					if (!(checkNode->x == nX && checkNode->y == nY))
					{
						newNode = new NODE();
						newNode->x = nX;
						newNode->y = nY;
						newNode->parent = checkNode;
						newNode->G = checkNode->G + 1;
						newNode->H = manhattan(nX, nY);
						newNode->F = newNode->G + newNode->H;
						map->setTile(OPEN, nX, nY);
						checkNode = newNode;
						pushOpenList(newNode);
					}
				}
			}

			if (wall[1])
			{
				if (map->getTile(nX - 2, nY) == ROAD)
				{
					if (!(checkNode->x == nX && checkNode->y == nY))
					{
						newNode = new NODE();
						newNode->x = nX;
						newNode->y = nY;
						newNode->parent = checkNode;
						newNode->G = checkNode->G + 1;
						newNode->H = manhattan(nX, nY);
						newNode->F = newNode->G + newNode->H;
						map->setTile(OPEN, nX, nY);
						checkNode = newNode;
						pushOpenList(newNode);
					}
				}
			}

			if (!(map->getTile(nX, nY) == START || map->getTile(nX, nY) == END))
				map->setTile(search, nX, nY);

			wall[0] = false;
			wall[1] = false;
			nX -= 1;
			nY += 1;

			if (map->getTile(nX, nY) == WALL || map->getTile(nX, nY) == WRONG)
				break;
		}
		break;
	case DIR_LU:
		while (1)
		{
			if (map->getTile(nX, nY - 1) == ROAD)
			{
				map->setTile(search, nX, nY - 1);
			}
			else if (map->getTile(nX, nY - 1) == WALL)
			{
				wall[0] = true;
			}

			if (map->getTile(nX - 1, nY) == ROAD)
			{
				map->setTile(search, nX - 1, nY);
			}
			else if (map->getTile(nX - 1, nY) == WALL)
			{
				wall[1] = true;
			}

			if (wall[0])
			{
				if (map->getTile(nX, nY - 2) == ROAD)
				{
					if (!(checkNode->x == nX && checkNode->y == nY))
					{
						newNode = new NODE();
						newNode->x = nX;
						newNode->y = nY;
						newNode->parent = checkNode;
						newNode->G = checkNode->G + 1;
						newNode->H = manhattan(nX, nY);
						newNode->F = newNode->G + newNode->H;
						map->setTile(OPEN, nX, nY);
						checkNode = newNode;
						pushOpenList(newNode);
					}
				}
			}

			if (wall[1])
			{
				if (map->getTile(nX - 2, nY) == ROAD)
				{
					if (!(checkNode->x == nX && checkNode->y == nY))
					{
						newNode = new NODE();
						newNode->x = nX;
						newNode->y = nY;
						newNode->parent = checkNode;
						newNode->G = checkNode->G + 1;
						newNode->H = manhattan(nX, nY);
						newNode->F = newNode->G + newNode->H;
						map->setTile(OPEN, nX, nY);
						checkNode = newNode;
						pushOpenList(newNode);
					}
				}
			}

			if (!(map->getTile(nX, nY) == START || map->getTile(nX, nY) == END))
				map->setTile(search, nX, nY);

			wall[0] = false;
			wall[1] = false;
			nX -= 1;
			nY -= 1;

			if (map->getTile(nX, nY) == WALL || map->getTile(nX, nY) == WRONG)
				break;
		}
		break;
	case DIR_RU:
		while (1)
		{
			if (map->getTile(nX, nY - 1) == ROAD)
			{
				map->setTile(search, nX, nY - 1);
			}
			else if (map->getTile(nX, nY - 1) == WALL)
			{
				wall[0] = true;
			}

			if (map->getTile(nX + 1, nY) == ROAD)
			{
				map->setTile(search, nX + 1, nY);
			}
			else if (map->getTile(nX + 1, nY) == WALL)
			{
				wall[1] = true;
			}

			if (wall[0])
			{
				if (map->getTile(nX, nY - 2) == ROAD)
				{
					if (!(checkNode->x == nX && checkNode->y == nY))
					{
						newNode = new NODE();
						newNode->x = nX;
						newNode->y = nY;
						newNode->parent = checkNode;
						newNode->G = checkNode->G + 1;
						newNode->H = manhattan(nX, nY);
						newNode->F = newNode->G + newNode->H;
						map->setTile(OPEN, nX, nY);
						checkNode = newNode;
						pushOpenList(newNode);
					}
				}
			}

			if (wall[1])
			{
				if (map->getTile(nX + 2, nY) == ROAD)
				{
					if (!(checkNode->x == nX && checkNode->y == nY))
					{
						newNode = new NODE();
						newNode->x = nX;
						newNode->y = nY;
						newNode->parent = checkNode;
						newNode->G = checkNode->G + 1;
						newNode->H = manhattan(nX, nY);
						newNode->F = newNode->G + newNode->H;
						map->setTile(OPEN, nX, nY);
						checkNode = newNode;
						pushOpenList(newNode);
					}
				}
			}

			if (!(map->getTile(nX, nY) == START || map->getTile(nX, nY) == END))
				map->setTile(search, nX, nY);

			wall[0] = false;
			wall[1] = false;
			nX += 1;
			nY -= 1;

			if (map->getTile(nX, nY) == WALL || map->getTile(nX, nY) == WRONG)
				break;
		}
		break;
	case DIR_RD:
		while (1)
		{
			if (map->getTile(nX, nY + 1) == ROAD)
			{
				map->setTile(search, nX, nY + 1);
			}
			else if (map->getTile(nX, nY + 1) == WALL)
			{
				wall[0] = true;
			}

			if (map->getTile(nX + 1, nY) == ROAD)
			{
				map->setTile(search, nX + 1, nY);
			}
			else if (map->getTile(nX + 1, nY) == WALL)
			{
				wall[1] = true;
			}

			if (wall[0])
			{
				if (map->getTile(nX, nY + 2) == ROAD)
				{
					if (!(checkNode->x == nX && checkNode->y == nY))
					{
						newNode = new NODE();
						newNode->x = nX;
						newNode->y = nY;
						newNode->parent = checkNode;
						newNode->G = checkNode->G + 1;
						newNode->H = manhattan(nX, nY);
						newNode->F = newNode->G + newNode->H;
						map->setTile(OPEN, nX, nY);
						checkNode = newNode;
						pushOpenList(newNode);
					}
				}
			}

			if (wall[1])
			{
				if (map->getTile(nX + 2, nY) == ROAD)
				{
					if (!(checkNode->x == nX && checkNode->y == nY))
					{
						newNode = new NODE();
						newNode->x = nX;
						newNode->y = nY;
						newNode->parent = checkNode;
						newNode->G = checkNode->G + 1;
						newNode->H = manhattan(nX, nY);
						newNode->F = newNode->G + newNode->H;
						map->setTile(OPEN, nX, nY);
						checkNode = newNode;
						pushOpenList(newNode);
					}
				}
			}

			if (!(map->getTile(nX, nY) == START || map->getTile(nX, nY) == END))
				map->setTile(search, nX, nY);

			wall[0] = false;
			wall[1] = false;
			nX += 1;
			nY += 1;

			if (map->getTile(nX, nY) == WALL || map->getTile(nX, nY) == WRONG)
				break;
		}
		break;
	}
	
}
*/
int JPS::getDistance(int x, int y, int dir)
{
	int distance=1;
	int temp;
	while (1)
	{
		temp = map->getTile(x, y);
		if (temp == WALL || temp == WRONG)
		{
			break;
		}
		distance++;
		//x += direction[dir][0];
		//y += direction[dir][1];
	}
	return distance;
}

int JPS::manhattan(int x, int y)
{
	int dx = x - map->GetEnd().x;
	int dy = y - map->GetEnd().y;

	if (dx < 0)
		dx *= -1;
	if (dy < 0)
		dy *= -1;

	return dx + dy;
}

bool JPS::pushOpenList(NODE* node)
{
	bool last = true;
	bool check = false;
	//저장 위치설정

	for (auto iter = openList.begin(); iter != openList.end(); iter++)
	{
		if ((*iter)->x == node->x && (*iter)->y == node->y)
		{
			if ((*iter)->G > node->G)
			{
				(*iter)->parent = node;
				(*iter)->G = node->G + 1;
			}
			check = true;
			delete node;
			return false;
		}
	}

	for (auto iter = closeList.begin(); iter != closeList.end(); iter++)
	{
		if ((*iter)->x == node->x && (*iter)->y == node->y)
		{
			if ((*iter)->G > node->G)
			{
				(*iter)->parent = node;
				(*iter)->G = node->G + 1;
			}
			check = true;
			delete node;
			return false;
		}
	}


	for (auto iter = openList.begin(); iter != openList.end(); iter++)
	{
		if (node->F < (*iter)->F)
		{
			openList.insert(iter, node);
			last = false;
			break;
		}
	}
	if (last)
	{
		openList.push_back(node);
	}
	int tile = map->getTile(node->x, node->y);
	if (tile != START && tile != END)
		map->setTile(OPEN, node->x, node->y);

	return true;
}

bool JPS::draw(BYTE *dib, int pitch, int blockSize)
{
	map->draw(dib, pitch, blockSize);

	if (successFind)
	{
		int oldX;
		int oldY;

		if (closeList.empty())
			return true;

		NODE *pathNode = (*closeList.begin());

		oldX = pathNode->x;
		oldY = pathNode->y;

		while (pathNode->parent !=NULL)
		{
			pathNode = pathNode->parent;

			bresenhamDraw(oldX,oldY, pathNode->x, pathNode->y);

			oldX = pathNode->x;
			oldY = pathNode->y;
		}
	}
}

bool JPS::bresenhamDraw(int sX, int sY, int fX, int fY)
{
	//if (sX > fX)
	//{
	//	int temp = sX;
	//	sX = fX;
	//	fX = temp;
	//
	//	temp = sY;
	//	sY = fY;
	//	fY = temp;
	//}

	int x = sX;
	int y = sY;

	int W = abs(sX - fX);
	int H = abs(sY - fY);

	if (W >= H)
	{
		if (sX > fX)
		{
			int temp = sX;
			sX = fX;
			fX = temp;

			temp = sY;
			sY = fY;
			fY = temp;
		}
		H = sY - fY;

		x = sX;
		y = sY;

		int F = 2 * abs(H) - W;
		int dF1 = 2 * abs(H);
		int dF2 = 2 * (abs(H) - W);

		for (x = sX; x <= fX; ++x)
		{
			map->setTile(PATH, x, y);

			if (F < 0)
			{
				F += dF1;
			}
			else
			{
				if (H < 0)
					++y;
				else
					--y;
				F += dF2;
			}
		}
	}
	else
	{
		if (sY > fY)
		{
			int temp = sX;
			sX = fX;
			fX = temp;

			temp = sY;
			sY = fY;
			fY = temp;
		}
		W = sX - fX;

		x = sX;
		y = sY;

		int F = 2 * abs(W) - H;
		int dF1 = 2 * abs(W);
		int dF2 = 2 * (abs(W) - H);

		for (y = sY; y <= fY; ++y)
		{
			map->setTile(PATH, x, y);

			if (F < 0)
			{
				F += dF1;
			}
			else
			{
				if (W < 0)
					++x;
				else
					--x;
				F += dF2;
			}
		}
	}

	
	
	/*
	// H/W는(기울기)0과 1사이로 가정
	// 처음 찍을 점은 시작점으로 한다.
	int x = _startX;
	int y = _startY;
	// W와 H를 구해줌.

	int W = _FinishX - _startX;
	int H = _FinishY - _startY;

	// 초기값
	int F = 2 * H - W;

	// 각 판별식공식

	int dF1 = 2 * H;
	int dF2 = 2 * (H - W);
	for (x = _startX; x <= _FinishX; ++x)
	{

		// 첫 시작점 그리기

		map->setTile(PATH, x, y);
		//setPixel(x, y);

		// 중단점이 0보다 작으면 그에 맞는 공식으로 판별식 갱신, y값은 그대로
		if (F < 0)
		{
			F += dF1;
		}

		else// 중단점이 0보다 크거나 같으면 그에 맞는 공식으로 판별식 갱신, y값은 증가
		{
			++y;
			F += dF2;
		}
	}
	*/

	return true;

}

bool JPS::bresenham(int sX, int sY, int fX, int fY)
{

	int x = sX;
	int y = sY;

	int W = abs(sX - fX);
	int H = abs(sY - fY);

	if (W >= H)
	{
		if (sX > fX)
		{
			int temp = sX;
			sX = fX;
			fX = temp;

			temp = sY;
			sY = fY;
			fY = temp;
		}
		H = sY - fY;

		x = sX;
		y = sY;

		int F = 2 * abs(H) - W;
		int dF1 = 2 * abs(H);
		int dF2 = 2 * (abs(H) - W);

		for (x = sX; x <= fX; ++x)
		{
			if (map->getTile(x, y) == WALL || map->getTile(x, y) == WRONG)
				return false;

			if (F < 0)
			{
				F += dF1;
			}
			else
			{
				if (H < 0)
					++y;
				else
					--y;
				F += dF2;
			}
		}
	}
	else
	{
		if (sY > fY)
		{
			int temp = sX;
			sX = fX;
			fX = temp;

			temp = sY;
			sY = fY;
			fY = temp;
		}
		W = sX - fX;

		x = sX;
		y = sY;

		int F = 2 * abs(W) - H;
		int dF1 = 2 * abs(W);
		int dF2 = 2 * (abs(W) - H);

		for (y = sY; y <= fY; ++y)
		{
			if (map->getTile(x, y) == WALL || map->getTile(x, y) == WRONG)
				return false;

			if (F < 0)
			{
				F += dF1;
			}
			else
			{
				if (W < 0)
					++x;
				else
					--x;
				F += dF2;
			}
		}
	}

	return true;

}

void JPS::correctionPath()
{
	NODE *checkNode = (*closeList.begin());

	if (checkNode->parent == NULL)
		return;

	NODE *jumpNode = checkNode->parent->parent;

	while (jumpNode!=NULL)
	{
		if (bresenham(checkNode->x, checkNode->y, jumpNode->x, jumpNode->y))
		{
			checkNode->parent = jumpNode;
			jumpNode = jumpNode->parent;
			
		}
		else
		{
			checkNode = checkNode->parent;
			jumpNode = checkNode->parent->parent;
		}
	}
}

void JPS::resetMap()
{
	resetList();

	map->resetMap();
	successFind = false;
}
void JPS::resetFind()
{
	resetList();
	map->resetFind();
	successFind = false;
}

void JPS::resetList()
{
	NODE *pNode;
	while (!openList.empty())
	{
		pNode = openList.front();
		openList.pop_front();
		delete pNode;
	}

	while (!closeList.empty())
	{
		pNode = closeList.front();
		closeList.pop_front();
		delete pNode;
	}
}