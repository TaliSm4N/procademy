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

		double G;//�̵�Ƚ��
		double H;//���������� �Ÿ�
		double F;//G+F

		NODE *parent;
	};

	JPS(int w,int h);
	~JPS();
	int Find();
	Map *GetMap() const { return map; };
private:
	bool searchStraight(int dir,NODE *checkNode,int &cX,int &cY);
	int manhattan(int x, int y);
	int getDistance(int x,int y,int dir);
private:
	std::list<NODE *> openList;
	std::list<NODE *> closeList;
	Map *map;
	bool successFind;
};