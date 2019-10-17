#pragma once

#include <Windows.h>

#define dfSCREEN_WIDTH		61		// 콘솔 가로 24칸 + NULL
#define dfSCREEN_HEIGHT		40		// 콘솔 세로 80칸

class Screen
{
public:
	static Screen *GetInstance();
	void flip();
	void clear();
	void draw(int iX, int iY, char chSprite);
private:
	Screen();
	void cs_MoveCursor(int iPosX, int iPosY);
	static Screen *screen;
	HANDLE hConsole;
	char szScreenBuffer[dfSCREEN_HEIGHT][dfSCREEN_WIDTH];
};

