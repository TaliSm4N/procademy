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
	// ���ϴ� ��ġ�� Ŀ���� �̵���Ų��.
	//-------------------------------------------------------------
	SetConsoleCursorPosition(hConsole, stCoord);
}

Screen::Screen()
{
	CONSOLE_CURSOR_INFO stConsoleCursor;
	system("mode con cols=60 lines=40");

	//-------------------------------------------------------------
	// ȭ���� Ŀ���� �Ⱥ��̰Բ� �����Ѵ�.
	//-------------------------------------------------------------
	stConsoleCursor.bVisible = FALSE;
	stConsoleCursor.dwSize = 1;			// Ŀ�� ũ��.
											// �̻��ϰԵ� 0 �̸� ���´�. 1���ϸ� �ȳ��´�.

	//-------------------------------------------------------------
	// �ܼ�ȭ�� (���ٴٵ� �ƿ�ǲ) �ڵ��� ���Ѵ�.
	//-------------------------------------------------------------
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorInfo(hConsole, &stConsoleCursor);
}

Screen *Screen::screen = nullptr;