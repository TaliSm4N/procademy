#pragma once
#pragma comment(lib, "winmm.lib")
#include "Camera.h"

class ScreenDib
{
public:
	ScreenDib(int iWidth, int iHeight, int iColorBit);
	virtual ~ScreenDib();

	BYTE *GetDibBuffer() const;
	int GetWidth() const;
	int GetHeight() const;
	int GetPitch() const;
	void Flip(HWND hWnd, int iX = 0, int iY = 0);
	Camera *GetCamera() { return cam; };

private:
	void CreateDibBuffer();
	void ReleaseDibBuffer();

	BITMAPINFO _stDibInfo;
	BYTE *_bypBuffer;

	int _iWidth;
	int _iHeight;
	int _iPitch;
	int _iColorBit;
	int _iBufferSize;

	Camera *cam;
};