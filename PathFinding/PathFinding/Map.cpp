#include "Map.h"

Map::Map(int w,int h)
	:width(w),height(h)
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

void Map::setTile(MODE mode, int x, int y)
{
	bool out = false;
	if (mode == ROAD)
	{
		tile[y][x] = 0;
	}
	else if (tile[y][x] == 0)
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
	if ((x < 0 || x >= width) && (y < 0 || y >= height))
		return WRONG;

	return (tile[y][x]);
}

bool Map::draw(BYTE *dib,int pitch,int blockSize)
{
	DWORD *dest = (DWORD *)dib;
	DWORD color = 0x00000000;

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			switch (tile[i][j])
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

	//draw grid
	for (int i = 1; i < height*blockSize; i++)
	{
		for (int j = 1; j < width*blockSize; j++)
		{
			if (i % 20 == 0 || j % 20 == 0)
				*(dest + j + (pitch / 4)*i) = 0x00000000;
		}
	}

	return true;
}