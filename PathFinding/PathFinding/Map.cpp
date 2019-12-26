#include "Map.h"

Map::Map(int w,int h)
	:width(w),height(h),successFind(false)
{
	tile = new int *[h];
	

	for (int i = 0; i < h; i++)
	{
		tile[i] = new int[w];
		ZeroMemory(tile[i], w*sizeof(int));
	}
}

Map::~Map()
{
	for (int i = 0; i < height; i++)
	{
		delete[] tile[height];
	}

	delete[] tile;
}

void Map::setTile(int mode, int x, int y)
{
	bool out = false;
	if (mode == ROAD)
	{
		tile[y][x] = 0;
	}
	else if (tile[y][x] == 0||tile[y][x]>=SEARCH||tile[y][x]==OPEN||tile[y][x]==CLOSE)
	{
		if (mode == START || mode == END)
		{
			for (int i = 0; i < height; i++)
			{
				for (int j = 0; j < width; j++)
				{
					if (tile[i][j] == mode)
					{
						tile[i][j] = 0;
						out = true;
						break;
					}
				}
				if (out)
					break;
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
		tile[y][x] = mode;
	}
}
int Map::getTile(int x, int y)
{
	if ((x < 0 || x >= width) || (y < 0 || y >= height))
		return WRONG;

	//if (tile[y][x] >= SEARCH)
	//	return ROAD;

	return (tile[y][x]);
}

bool Map::draw(BYTE *dib,int pitch,int blockSize)
{
	DWORD *dest = (DWORD *)dib;
	DWORD color = 0x00000000;
	static DWORD pathColor = 0xFFC0CB;
	bool test = false;

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			switch (tile[i][j])
			{
			case ROAD:
				color = 0x00ffffff;
				break;
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
			case PATH:
				color = pathColor;
				test = true;
				break;
			default:
				switch (tile[i][j] % 5)
				{
				case 0:
					color = 0x00ff00ff;
					break;
				case 1:
					color = 0x00ff5500;
					break;
				case 2:
					color = 0x0000ffff;
					break;
				case 3:
					color = 0x00751267;
					break;
				case 4:
					color = 0x00126344;
					break;
				}
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

	//draw grid
	for (int i = 1; i < height*blockSize; i++)
	{
		for (int j = 1; j < width*blockSize; j++)
		{
			if (i % 20 == 0 || j % 20 == 0)
				*(dest + j + (pitch / 4)*i) = 0x00000000;
		}
	}

	if (test)
		pathColor++;

	return true;
}

void Map::resetMap()
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
			tile[i][j] = 0;
	}
	successFind = false;
}

void Map::resetFind()
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			if (tile[i][j] != START && tile[i][j] != END && tile[i][j] != WALL)
				tile[i][j] = 0;
		}
	}
	successFind = false;
}