#include "JPS.h"
#define inMap

//int direction[8][2] = { {1,0} ,{0,1},{-1,0},{0,-1} ,{-1,1},{-1,-1},{1,-1},{1,1} };


enum Direction {DIR_LL=0,DIR_RR,DIR_DD,DIR_UU,DIR_LU,DIR_LD,DIR_RU,DIR_RD};


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
	NODE *checkNode;

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
			return 1;
		}
	}

	for (int i = 0; i < 8; i++)
	{
		int cX;
		int cY;
		if (i < 4)
			searchStraight(i, checkNode, cX, cY);
		
	}

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

bool JPS::searchStraight(int dir, NODE *checkNode,int &cX,int &cY)
{
	if (dir > 4)
		return false;

	int nX = checkNode->x;
	int nY = checkNode->y;

	int checkDir[2] = { (dir + 1) % 4,(dir + 3) % 4 };

	//if (map->getTile(nX + direction[checkDir[0]][0], nY + direction[checkDir[0]][1])==ROAD)
	//{
	//	//확인 체크
	//}
	//
	//if (map->getTile(nX + direction[checkDir[1]][0], nY + direction[checkDir[1]][1]) == ROAD)
	//{
	//	//확인 체크
	//}
	//
	//nX += direction[dir][0];
	//nY += direction[dir][1];
	//
	//while (map->getTile(nX,nY)==ROAD)
	//{
	//	nX += direction[dir][0];
	//	nY += direction[dir][1];
	//}
}

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
		x += direction[dir][0];
		y += direction[dir][1];
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