#pragma once
#include <Windows.h>

BOOL init();
BOOL Draw(HWND hWnd);
int GameFrame(HWND hWnd,bool active);

BOOL keyboard();
BOOL Run();

BOOL CreatePlayer(int id,int dir,int x,int y,char hp,bool player);

BOOL DeletePlayer(int id);

BOOL MovePlayer(int id, int dir,short x,short y);
BOOL StopPlayer(int id, int dir, short x, short y);
BOOL Attack1Player(int id, int dir, short x, short y);
BOOL Attack2Player(int id, int dir, short x, short y);
BOOL Attack3Player(int id, int dir, short x, short y);
BOOL DamagePlayer(int AttackID, int DamageID, char DamageHP);