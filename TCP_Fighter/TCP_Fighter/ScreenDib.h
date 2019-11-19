#pragma once
#include <Windows.h>

class ScreenDib
{
public:
	ScreenDib(int iWidth, int iHeight, int iColorBit);
	virtual ~ScreenDib();

	BYTE *GetDibBuffer();
	int GetWidth();
	int GetHeight();
	int GetPitch();
	void Flip(HWND hWnd, int iX = 0, int iY = 0);

private:
	void CreateDibBuffer(int iWidth, int iHeight, int iColorBit);
	void ReleaseDibBuffer();

	BITMAPINFO _stDibinfo;
	BYTE *_bypBuffer;

	int _iWidth;
	int _iHeight;
	int _iPitch;
	int _iColorBit;
	int _iBufferSize;

};