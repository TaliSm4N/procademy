#pragma once
#include <queue>
#include <list>
#include <Windows.h>


enum MODE { DELETE_WALL = 0, START = 1, END = 2, WALL = 3, OPEN=4,CLOSE=5 };



class AStar
{
public:
	AStar(int h, int w);
	~AStar();
	void setTile(MODE mode, int x, int y);
	MODE getTile(int x, int y);
	int Find();//-1 실패 0 찾는중 1 성공
	bool draw(BYTE *dib, int pitch,int blockSize);
	
	void resetMap();
	void resetFind();

	void TestMode();
	void ShowMode();

public:
	struct NODE
	{
		int x;
		int y;
		NODE *parent;
		double G;//이동횟수
		double H;//목적지와의 거리
		double F;//G+F

		bool operator < (NODE a)
		{
			return this->F < a.F;
		}

		bool operator > (NODE a)
		{
			return this->F > a.F;
		}

		bool operator <= (NODE a)
		{
			return this->F <= a.F;
		}

		bool operator >= (NODE a)
		{
			return this->F >= a.F;
		}
	};

	struct cmp
	{
		constexpr bool operator()(const NODE * &a, const NODE * &b)
		{
			return a->F > b->F;
		}
	};

	struct Pos
	{
		int x;
		int y;
	};
private:
	int manhattan(int x, int y);
	void writeLine(BYTE *dib,int blockSize,int pitch, NODE *sNode, NODE *eNode,DWORD color);
	void drawGrid(BYTE *dib, int blockSize, int pitch);
	void resetList();
private:
	//std::priority_queue<NODE *,std::vector<NODE *>,cmp> openList;
	std::list<NODE *> openList;
	std::list<NODE *> closeList;
	//int **map;
	
	int width;
	int height;
	Pos start;
	Pos end;
	bool successFind;
	char testToken;
	bool showToken;
};