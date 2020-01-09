#pragma once
#include <windows.h>
#include "ScreenDib.h"
//#include "stdafx.h"

//class ScreenDib
//{
//public:
//	ScreenDib(int iWidth, int iHeight, int iColorBit);
//	virtual ~ScreenDib();
//
//	BYTE *GetDibBuffer() const;
//	int GetWidth() const;
//	int GetHeight() const;
//	int GetPitch() const;
//	void Flip(HWND hWnd, int iX = 0, int iY = 0);
//
//private:
//	void CreateDibBuffer(int iWidth, int iHeight, int iColorBit);
//	void ReleaseDibBuffer();
//
//	BITMAPINFO _stDibinfo;
//	BYTE *_bypBuffer;
//
//	int _iWidth;
//	int _iHeight;
//	int _iPitch;
//	int _iColorBit;
//	int _iBufferSize;
//
//};

ScreenDib::ScreenDib(int iWidth, int iHeight, int iColorBit)
	:_iWidth(iWidth),_iHeight(iHeight),_iColorBit(iColorBit)
{
	CreateDibBuffer();
	cam = new Camera(_iWidth, _iHeight);
}

ScreenDib::~ScreenDib()
{
	ReleaseDibBuffer();
}

void ScreenDib::CreateDibBuffer()
{
	memset(&_stDibInfo, 0, sizeof(BITMAPINFO));
	_stDibInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	_stDibInfo.bmiHeader.biWidth = _iWidth;
	_stDibInfo.bmiHeader.biHeight = -_iHeight;
	_stDibInfo.bmiHeader.biPlanes = 1;
	_stDibInfo.bmiHeader.biBitCount = _iColorBit;
	_stDibInfo.bmiHeader.biCompression = 0;
	_stDibInfo.bmiHeader.biSizeImage = _iBufferSize;
	_stDibInfo.bmiHeader.biXPelsPerMeter = 0;
	_stDibInfo.bmiHeader.biYPelsPerMeter = 0;
	_stDibInfo.bmiHeader.biClrUsed = 0;
	_stDibInfo.bmiHeader.biClrImportant = 0;

	_iPitch = ((_iWidth * (_iColorBit / 8)) + 3) & ~3;
	_iBufferSize = _iPitch * _iHeight;

	_bypBuffer = new BYTE[_iBufferSize];
	memset(_bypBuffer, 0xff, _iBufferSize);
}
void ScreenDib::ReleaseDibBuffer()
{
	if(_bypBuffer!=NULL)
		delete[] _bypBuffer;
	_bypBuffer = NULL;
	memset(&_stDibInfo, 0, sizeof(BITMAPINFO));
	_iWidth = 0;
	_iHeight = 0;
	_iColorBit = 0;

	_iPitch = 0;
	_iBufferSize = 0;

	
}

void ScreenDib::Flip(HWND hWnd, int iX, int iY)
{
	if (_bypBuffer == NULL) return;

	//RECT Rect;
	HDC hDC = GetDC(hWnd);
	SetDIBitsToDevice(hDC,
		0, 0,
		_iWidth, _iHeight,
		0, 0,
		0, _iHeight,

		_bypBuffer,
		&_stDibInfo, DIB_RGB_COLORS);


	//frame set
	static char szFrame[5];
	static int iFrame = 0;
	static DWORD dwTick = 0;
	iFrame++;
	if (dwTick + 1000 < timeGetTime())
	{
		wsprintfA(szFrame, "%d", iFrame);
		iFrame = 0;
		dwTick = timeGetTime();
	}

	TextOutA(hDC, 0, 0, szFrame, (int)strlen(szFrame));

	ReleaseDC(hWnd, hDC);
}


BYTE *ScreenDib::GetDibBuffer() const
{
	return _bypBuffer;
}

int ScreenDib::GetHeight() const
{
	return _iHeight;
}

int ScreenDib::GetPitch() const
{
	return _iPitch;
}

int ScreenDib::GetWidth() const
{
	return _iWidth;
}