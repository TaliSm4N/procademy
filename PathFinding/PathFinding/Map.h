#pragma once
#include <Windows.h>

enum MODE { ROAD = 0, START = 1, END = 2, WALL = 3, OPEN = 4, CLOSE = 5,WRONG = 6 };

class Map
{
private:
	struct Pos
	{
		int x;
		int y;
	};
public:
	//enum MODE { DELETE_WALL = 0, START = 1, END = 2, WALL = 3, OPEN = 4, CLOSE = 5 };
	Map(int w,int h);
	~Map();
	void setTile(MODE mode, int x, int y);
	int getTile(int x, int y);
	bool draw(BYTE *dib, int pitch, int blockSize);
	Pos GetStart() const { return start; }
	Pos GetEnd() const { return end; }
private:
	int **tile;
	int width;
	int height;
	Pos start;
	Pos end;
};