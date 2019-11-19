#include "ScreenDib.h"

ScreenDib::ScreenDib(int iWidth, int iHeight, int iColorBit)
	:_iWidth(iWidth),_iHeight(iHeight),_iColorBit(iColorBit)
{
	CreateDibBuffer(iWidth, iHeight, iColorBit);

	int _iPitch = (_iWidth * (_iColorBit/8) + 3)&~3;
}

ScreenDib::~ScreenDib()
{
	ReleaseDibBuffer();
}

BYTE *ScreenDib::GetDibBuffer()
{
	return _bypBuffer;
}

int ScreenDib::GetWidth()
{
	return _iWidth;
}

int ScreenDib::GetHeight()
{
	return _iHeight;
}

int ScreenDib::GetPitch()
{
	return _iPitch;
}

void ScreenDib::Flip(HWND hWnd, int iX, int iY)
{
}

void ScreenDib::CreateDibBuffer(int iWidth, int iHeight, int iColorBit)
{
	_stDibinfo.bmiHeader.biSize=0x28;//40
	_stDibinfo.bmiHeader.biWidth = 640;
	_stDibinfo.bmiHeader.biHeight = 480;
	_stDibinfo.bmiHeader.biPlanes = 1;
	_stDibinfo.bmiHeader.biBitCount=32;
	_stDibinfo.bmiHeader.biCompression = BI_RGB;
	_stDibinfo.bmiHeader.biSizeImage= 640 * 480 * (32 / 8);
	//_stDibinfo.bmiHeader.biXPelsPerMeter;
	//_stDibinfo.bmiHeader.biYPelsPerMeter;
	//_stDibinfo.bmiHeader.biClrUsed;
	//_stDibinfo.bmiHeader.biClrImportant; 
	


}

void ScreenDib::ReleaseDibBuffer()
{
	delete _bypBuffer;
}