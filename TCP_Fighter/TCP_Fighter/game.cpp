#include "game.h"

ScreenDib g_ScreenDib(640, 480, 32);
HWND g_hWnd;

void Update()
{
	BYTE *pDest = g_ScreenDib.GetDibBuffer();
	int iWidth = g_ScreenDib.GetWidth();
	int iHeight = g_ScreenDib.GetHeight();
	int iPitch = g_ScreenDib.GetPitch();

	BYTE byGrayColor = 0;

	for (int iCnt = 0; iCnt < 480; ++iCnt)
	{
		memset(pDest, byGrayColor, 4 * 640);
		pDest += iPitch;
		byGrayColor++;
	}

	g_ScreenDib.Flip(g_hWnd, 0, 0);
}

void initGame(HWND hWnd)
{
	g_hWnd = hWnd;
}