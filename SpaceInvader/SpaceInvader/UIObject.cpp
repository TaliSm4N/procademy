#define _CRT_SECURE_NO_WARNINGS
#include "UIObject.h"
#include "PlayerObject.h"
#include <Windows.h>
#include <iostream>
#include <cstring>
#include "Screen.h"
#include "score.h"

StringObject::StringObject(int x, int y, const char *str) : BaseObject(DataType::UI, x, y)
{
	strcpy(_str, str);
}

bool StringObject::Action()
{
	if (GetAsyncKeyState(VK_SPACE))
		return true;
	else
		return false;
}

bool StringObject::Draw()
{
	int len = strlen(_str);
	for (int i = 0; i < len; i++)
	{
		Screen::GetInstance()->draw(_x+i, _y, _str[i]);
	}

	return true;
}

FrameObject::FrameObject():BaseObject(DataType::UI, 0, 0) {}

bool FrameObject::Action() { return true; }

bool FrameObject::Draw()
{
	for (int i = 0; i < dfSCREEN_HEIGHT; i++)
	{
		for (int j = 0; j < dfSCREEN_WIDTH; j++)
		{
			if (frame[i][j] != ' ')
				Screen::GetInstance()->draw(j, i, frame[i][j]);
		}
	}
	return true;
}

bool HUDObject::Action() { return true; }

bool HUDObject::Draw() 
{
	char stageWord[6] = "STAGE";
	Screen::GetInstance()->draw(2, _y, 'H');
	Screen::GetInstance()->draw(3, _y, 'P');
	Screen::GetInstance()->draw(5, _y, *_HP / 10 + 48);
	Screen::GetInstance()->draw(6, _y, *_HP % 10 + 48);
	Screen::GetInstance()->draw(7, _y, '/');
	Screen::GetInstance()->draw(8, _y, PLAYER_HP / 10 + 48);
	Screen::GetInstance()->draw(9, _y, PLAYER_HP % 10 + 48);
	
	int div = 1000000;
	char scoreWord[6] = "SCORE";
	for (int i = 0; i < 5; i++)
	{
		Screen::GetInstance()->draw(23 + i, _y, scoreWord[i]);
	}

	for (int i = 0; i < 7; i++)
	{
		Screen::GetInstance()->draw(31 + i, _y, (score / div) % 10 + 48);
		div /= 10;
	}

	for (int i = 0; i < 5; i++)
	{
		Screen::GetInstance()->draw(50 + i, _y, stageWord[i]);
	}
	Screen::GetInstance()->draw(56, _y, *_stage / 10 + 48);
	Screen::GetInstance()->draw(57, _y, *_stage % 10 + 48);
	return true;
}