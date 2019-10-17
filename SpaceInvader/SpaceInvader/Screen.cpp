#include "Screen.h"
#include <iostream>
#include <Windows.h>

Screen *Screen::GetInstance()
{
	if (screen == nullptr)
	{
		screen = new Screen();
	}

	return screen;
}

void Screen::flip()
{
	for (int i = 0; i < dfSCREEN_HEIGHT; i++)
	{
		cs_MoveCursor(0, i);
		printf("%s", szScreenBuffer[i]);

	}
}
void Screen::clear()
{
	memset(szScreenBuffer, ' ', dfSCREEN_HEIGHT*dfSCREEN_WIDTH);

	for (int i = 0; i < dfSCREEN_HEIGHT; i++)
	{
		szScreenBuffer[i][dfSCREEN_WIDTH - 1] = '\0';
	}
}
void Screen::draw(int iX, int iY, char chSprite)
{
	szScreenBuffer[iY][iX] = chSprite;
}

void Screen::cs_MoveCursor(int iPosX, int iPosY)
{
	COORD stCoord;
	stCoord.X = iPosX;
	stCoord.Y = iPosY;
	//-------------------------------------------------------------
	// 원하는 위치로 커서를 이동시킨다.
	//-------------------------------------------------------------
	SetConsoleCursorPosition(hConsole, stCoord);
}

Screen::Screen()
{
	CONSOLE_CURSOR_INFO stConsoleCursor;
	system("mode con cols=60 lines=40");

	//-------------------------------------------------------------
	// 화면의 커서를 안보이게끔 설정한다.
	//-------------------------------------------------------------
	stConsoleCursor.bVisible = FALSE;
	stConsoleCursor.dwSize = 1;			// 커서 크기.
											// 이상하게도 0 이면 나온다. 1로하면 안나온다.

	//-------------------------------------------------------------
	// 콘솔화면 (스텐다드 아웃풋) 핸들을 구한다.
	//-------------------------------------------------------------
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorInfo(hConsole, &stConsoleCursor);
}

Screen *Screen::screen = nullptr;