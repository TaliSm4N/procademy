#include "AStar.h"

int direction[8][2] = { {1,0} ,{0,1},{-1,0},{0,-1} ,{-1,1},{-1,-1},{1,-1},{1,1} };
int map[32][64] = {0,};
AStar::AStar(int w, int h)
	:width(w),height(h), successFind(false),testToken(false),showToken(false)
{
	//map = new int *[h]();
	//
	//for (int i = 0; i < w; i++)
	//{
	//	map[i] = new int[w];
	//	memset(map[i], 0, sizeof(int)*w);
	//}
	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
			map[i][j] = 0;
	}
}

AStar::~AStar()
{
	resetMap();
	//for (int i = 0; i < width; i++)
	//{
	//	delete map[i];
	//}
	//delete map;
}

void AStar::setTile(MODE mode, int x, int y)
{
	if (mode == DELETE_WALL)
	{
		map[y][x] = 0;
	}
	else if (map[y][x] == 0)
	{
		if (mode == START || mode == END)
		{
			for (int i = 0; i < height; i++)
			{
				for (int j = 0; j < width; j++)
				{
					if (map[i][j] == mode)
						map[i][j] = 0;
				}
			}
			if (mode == START)
			{
				start.x = x;
				start.y = y;
			}
			else if (mode == END)
			{
				end.x = x;
				end.y = y;
			}
		}
		map[y][x] = mode;
	}
}
MODE AStar::getTile(int x, int y)
{
	return (MODE)(map[y][x]);
}

//-1 실패 0 찾는중 1 성공
int AStar::Find()
{
	NODE *checkNode;
	NODE *newNode;
	int x;
	int y;
	double G;
	double H;
	double F;
	bool check = false;

	if (successFind)
		return 1;
	
	if (closeList.empty())//최초 실행
	{
		checkNode = new NODE();
		checkNode->x = start.x;
		checkNode->y = start.y;
		checkNode->parent = nullptr;
		checkNode->G = 0;
		checkNode->H = manhattan(start.x, start.y);
		checkNode->F = checkNode->G + checkNode->H;
		closeList.push_front(checkNode);
		//map[checkNode->y][checkNode->x] = OPEN;
	}
	else if (!openList.empty())
	{
		checkNode = openList.front();
		closeList.push_front(checkNode);
		openList.pop_front();
		if(map[checkNode->y][checkNode->x]!=START&& map[checkNode->y][checkNode->x]!=END)
			map[checkNode->y][checkNode->x] = CLOSE;
		if (checkNode->x == end.x&&checkNode->y == end.y)
		{
			successFind = true;
			return 1;
		}
	}
	else//더이상 찾을 수 없을 경우
		return -1;

	if (checkNode->x == end.x&&checkNode->y == end.y)
	{
		return 1;
	}

	for (int i = 0; i < 8; i++)
	{
		check = false;
		x = checkNode->x + direction[i][1];
		y = checkNode->y + direction[i][0];
		if(i<4)
			G = checkNode->G + 1;
		else
			G = checkNode->G + 1.5;
		H = manhattan(x, y);
		F = G + H;

		if (x < 0 || x >= width || y < 0 || y >= height)
			continue;

		if (map[y][x] == WALL)
			continue;

		for (auto iter = openList.begin(); iter != openList.end(); iter++)
		{
			if ((*iter)->x == x && (*iter)->y == y)
			{
				if ((*iter)->G > G)
				{
 					(*iter)->parent = checkNode;
					if (i < 4)
						(*iter)->G = checkNode->G + 1;
					else
						(*iter)->G = checkNode->G + 1/5;
				}
				check = true;
				break;
			}
		}

		if (check)
		{
			continue;
		}

		for (auto iter = closeList.begin(); iter != closeList.end(); iter++)
		{
			if ((*iter)->x == x && (*iter)->y == y)
			{
				if ((*iter)->G > G)
				{
					(*iter)->parent = checkNode;
					if (i < 4)
						(*iter)->G = checkNode->G + 1;
					else
						(*iter)->G = checkNode->G + 1.5;
				}
				check = true;
				break;
			}
		}

		if (check)
		{
			continue;
		}

		newNode = new NODE();

		newNode->F = F;
		newNode->G = G;
		newNode->H = H;
		newNode->x = x;
		newNode->y = y;
		newNode->parent = checkNode;

		bool last = true;
		//저장 위치설정
		for (auto iter = openList.begin(); iter != openList.end(); iter++)
		{
			if (F < (*iter)->F)
			{
				openList.insert(iter, newNode);
				last = false;
				break;
			}
		}
		if (last)
		{
			openList.push_back(newNode);
		}
		if (map[newNode->y][newNode->x] != START && map[newNode->y][newNode->x] != END)
			map[newNode->y][newNode->x] = OPEN;
	}

   	return 0;
	/*
	if (!openList.empty())
	{
		checkNode = openList.front();

		openList.pop_front();

		closeList.push_back(checkNode);

		

		
		checkNode = openList.front();


		for (int i = 0; i < 8; i++)
		{
			x = checkNode->x + direction[i][0];
			y = checkNode->y + direction[i][1];
			G = checkNode->G + 1;
			H = manhattan(x, y);
			F = G + H;

			if (x < 0 || x >= width)
				continue;
			if (y < 0 || y >= height)
				continue;

			if (map[y][x] == WALL)
				continue;
			else
			{
				NODE *newNode;
				check = false;

				//openList체크
				for (auto iter = openList.begin(); iter != openList.end(); iter++)
				{
					if ((*iter)->x == x && (*iter)->y == y)
					{
						if ((*iter)->F > F)
						{
							(*iter)->parent = checkNode;
						}
						check = true;
					}
					
				}

				if (check)
				{
					continue;
				}
				for (auto iter = closeList.begin(); iter != closeList.end(); iter++)
				{
					if ((*iter)->x == x && (*iter)->y == y)
					{
						if ((*iter)->F > F)
						{
							(*iter)->parent = checkNode;
						}
						check = true;
					}
				}

				if (check)
				{
					continue;
				}

				newNode = new NODE();
				newNode->x = x;
				newNode->y = y;
				newNode->G = G;
				newNode->H = H;
				newNode->F = F;
				newNode->parent = checkNode;
				map[newNode->y][newNode->x] = OPEN;
				bool last=true;
				//저장 위치설정
				for (auto iter = openList.begin(); iter != openList.end(); iter++)
				{
					if (F <= (*iter)->F)
					{
						openList.insert(iter, newNode);
						last = false;
						break;
					}
				}
				if (last)
				{
					openList.push_back(newNode);
				}
			}
		}
		openList.pop_front();
		closeList.push_front(checkNode);
		map[checkNode->y][checkNode->x] = CLOSE;
		
	}
	else
		return -1;
	*/
}

bool AStar::draw(BYTE *dib, int pitch,int blockSize)
{
	DWORD *dest = (DWORD *)dib;
	DWORD color = 0x00000000;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			switch (map[i][j])
			{
			case START:
				color = 0x0000ff00;
				break;
			case END:
				color = 0x00ff0000;
				break;
			case WALL:
				color = 0x00555555;
				break;
			case OPEN:
				color = 0x000000ff;
				break;
			case CLOSE:
				color = 0x00ffff00;
				break;
			default:
				color = 0x00ffffff;
				//return;
				break;
			}
			for (int iDraw = i * blockSize; iDraw < i*blockSize + blockSize; iDraw++)
			{
				for (int jDraw = j * blockSize; jDraw < j*blockSize + blockSize; jDraw++)
					*(dest + jDraw + (pitch / 4)*iDraw) = color;
			}
		}
	}

	drawGrid(dib, blockSize, pitch);

	

	if (testToken>0)
	{
		if (openList.empty())
			return true;

		for (auto iter = openList.begin(); iter != openList.end(); iter++)
		{
			writeLine(dib, blockSize, pitch, *iter, (*iter)->parent, 0xffc0cb);
		}
	}

	if (testToken > 1)
	{
		if (closeList.empty())
			return true;

		for (auto iter = closeList.begin(); iter != closeList.end(); iter++)
		{
			writeLine(dib, blockSize, pitch, *iter, (*iter)->parent,0x8b00ff );
		}
	}

	if (showToken || successFind)
	{
		int oldX;
		int oldY;

		if (closeList.empty())
			return true;

		NODE *pathNode = (*closeList.begin());

		oldX = pathNode->x;
		oldY = pathNode->y;

		while (pathNode != NULL)
		{
			//writeLine(dib, blockSize, pitch, pathNode, pathNode->parent, 0x00ff0000);
			pathNode = pathNode->parent;
		}
	}

	////if (successFind)
	//{
	//	int oldX;
	//	int oldY;
	//
	//	if (closeList.empty())
	//		return true;
	//
	//	NODE *pathNode = (*closeList.begin());
	//
	//	oldX = pathNode->x;
	//	oldY = pathNode->y;
	//	
	//	for (auto iter = closeList.begin(); iter != closeList.end(); iter++)
	//	{
	//		writeLine(dib, blockSize, pitch, *iter, (*iter)->parent);
	//	}
	//	
	//	//while (pathNode != NULL)
	//	//{
	//	//	writeLine(dib,blockSize,pitch, pathNode,pathNode->parent);
	//	//	pathNode = pathNode->parent;
	//	//}
	//
	//	
	//}

	return true;
}

void AStar::resetMap()
{
	resetList();

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
			map[i][j] = 0;
	}
	successFind = false;
}
void AStar::resetFind()
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			if(map[i][j]!=START&&map[i][j]!=END&&map[i][j]!=WALL)
				map[i][j] = 0;
		}
	}
	resetList();
	successFind = false;
}

void AStar::resetList()
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

int AStar::manhattan(int x, int y)
{
	int dx = x - end.x;
	int dy = y - end.y;

	if (dx < 0)
		dx *= -1;
	if (dy < 0)
		dy *= -1;

	return dx + dy;
}

void AStar::drawGrid(BYTE *dib, int blockSize, int pitch)
{
	DWORD *dest = (DWORD *)dib;
	for (int i = 1; i < height*blockSize; i++)
	{
		for (int j = 1; j < width*blockSize; j++)
		{
			if (i % 20 == 0 || j % 20 == 0)
				*(dest + j + (pitch / 4)*i) = 0x00000000;
		}
	}
}

void AStar::writeLine(BYTE *dib, int blockSize,int pitch, NODE *sNode, NODE *eNode,DWORD color)
{
	//DWORD color = 0x00ff0000;
	DWORD *dest = (DWORD *)dib;

	if (eNode == NULL)
		return;

	int xDir = eNode->x - sNode->x;
	int yDir = eNode->y - sNode->y;

	int xPos = sNode->x*blockSize + blockSize / 2;
	int yPos = sNode->y*blockSize + blockSize / 2;

	

	for (int i = 0; i < blockSize; i++)
	{
		*(dest + xPos + (pitch / 4)*yPos) = color;
		*(dest + xPos - 1 + (pitch / 4)*yPos) = color;
		*(dest + xPos - 2 + (pitch / 4)*yPos) = color;

		*(dest + xPos + (pitch / 4)*(yPos - 1)) = color;
		*(dest + xPos - 1 + (pitch / 4)*(yPos - 1)) = color;
		*(dest + xPos - 2 + (pitch / 4)*(yPos - 1)) = color;

		*(dest + xPos + (pitch / 4)*(yPos - 2)) = color;
		*(dest + xPos - 1 + (pitch / 4)*(yPos - 2)) = color;
		*(dest + xPos - 2 + (pitch / 4)*(yPos - 2)) = color;

		xPos += xDir;
		yPos += yDir;
	}
	//*(dest + jDraw + (pitch / 4)*iDraw) = color;
}

void AStar::TestMode()
{
	testToken++;
	testToken %= 3;
}
void AStar::ShowMode()
{
	showToken = !showToken;
}