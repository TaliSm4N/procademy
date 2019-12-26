#pragma once
#include "Map.h"
#include <list>

class JPS
{
public:
	struct NODE
	{
		int x;
		int y;

		double G;//이동횟수
		double H;//목적지와의 거리
		double F;//G+F
		int pDir;

		NODE *parent;
	};

	JPS(int w,int h);
	~JPS();
	int Find();
	Map *GetMap() const { return map; };
	bool draw(BYTE *dib, int pitch, int blockSize);
	void resetMap();
	void resetFind();
	void resetList();
private:
	//bool searchStraight(int dir,NODE *checkNode,int &cX,int &cY);
	bool searchStraight(int dir, int nX,int nY, int &cX, int &cY);
	bool searchDiagonal(int dir, NODE *checkNode, int &cX, int &cY);
	int manhattan(int x, int y);
	int getDistance(int x,int y,int dir);
	bool pushOpenList(NODE *node);
	bool bresenhamDraw(int _startX, int _startY, int _FinishX, int _FinishY);
	bool bresenham(int _startX, int _startY, int _FinishX, int _FinishY);
	void correctionPath();
private:
	std::list<NODE *> openList;
	std::list<NODE *> closeList;
	Map *map;
	bool successFind;
};