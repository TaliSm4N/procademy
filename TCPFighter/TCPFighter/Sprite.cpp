#include "Sprite.h"
#include <iostream>
#include "Camera.h"
/*
class SpriteList
{
public:
	SpriteList(int size = 10);
	~SpriteList();

	struct Sprite
	{
		BYTE	*bypImage;				// 스프라이트 이미지 포인터.
		int		iWidth;					// Widht
		int		iHeight;				// Height
		int		iPitch;					// Pitch

		int		iCenterPointX;			// 중점 X
		int		iCenterPointY;			// 중점 Y
	};
	bool settingSprite(int num, char *path, int cX, int cY);
private:
	int ListSize;
	Sprite *s_list;
};
*/



SpriteList::SpriteList(int size)
	:ListSize(size)
{
	s_list = new Sprite[ListSize];
	memset(s_list, 0, sizeof(Sprite)*ListSize);
}

SpriteList::~SpriteList()
{
	for (int i = 0; i < ListSize; i++)
	{
		releaseSprite(i);
	}

	delete[] s_list;
}

void SpriteList::releaseSprite(int num)
{
	if (num >= ListSize)
		return;

	if (s_list[num].bypImage != NULL)
	{
		delete[]  s_list[num].bypImage;
		memset(&s_list[num], 0, sizeof(Sprite));
	}
}

bool SpriteList::settingSprite(int num, const char *path, int cX, int cY)
{

	if (num >= ListSize)
		return false;

	FILE *f = fopen(path, "rb");

	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;


	if (f == nullptr)
	{
		return false;
	}

	int ret = fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, f);

	if (ret == 0)
	{
		return false;
	}

	fseek(f, sizeof(BITMAPFILEHEADER), SEEK_SET);

	if (0x4d42 == fileHeader.bfType)
	{
		ret = fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, f);
		fseek(f, sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER), SEEK_SET);
		if (32 == infoHeader.biBitCount)
		{
			
			s_list[num].iCenterPointX = cX;
			s_list[num].iCenterPointY = cY;
			s_list[num].iHeight = infoHeader.biHeight;
			s_list[num].iPitch = (infoHeader.biWidth * 4) + 3 & ~3;
			s_list[num].iWidth = infoHeader.biWidth;
			int size = s_list[num].iPitch * s_list[num].iHeight;
			s_list[num].bypImage = new BYTE[size];

			//뒤집어서 저장
			for (int i = 0; i < s_list[num].iHeight; i++)
			{
				fread(s_list[num].bypImage+(s_list[num].iHeight-i-1)*s_list[num].iPitch, s_list[num].iPitch, 1, f);
			}
			

			fclose(f);
			return true;
		}
	}

	fclose(f);

	return false;
}

bool SpriteList::draw(int num, BYTE *dib, int x, int y, int width, int height,int pitch,int len)
{
	if (num >= ListSize)
		return false;

	if (s_list[num].bypImage == NULL)
		return false;
	
	Sprite *sprite = &s_list[num];
	
	int spWidth = sprite->iWidth*len/100;
	int spHeight = sprite->iHeight;
	
	DWORD *dest = (DWORD *)dib;
	DWORD *img = (DWORD *)sprite->bypImage;
	
	x -= sprite->iCenterPointX;
	y -= sprite->iCenterPointY;
	
	if (y < 0)
	{
		spHeight += y;
		img = (DWORD *)(sprite->bypImage + sprite->iPitch*(-y));
		y = 0;
	}
	
	if (y + sprite->iHeight >= height)
	{
		spHeight -= y + sprite->iHeight - height;
	}
	
	if (x < 0)
	{
		spWidth += x;
		img -= x;
		x = 0;
	}
	
	if (x + sprite->iWidth >= width)
	{
		spWidth -= x + sprite->iWidth - width;
	}
	
	if (spWidth <= 0 || spHeight <= 0)
		return false;

	for (int i = 0; i < spHeight; i++)
	{
		for (int j = 0; j < spWidth; j++)
		{
			if ((*(img + j + (sprite->iPitch / 4)*i) & 0x00ffffff) != 0x00ffffff)
			{
				*(dest+j+x+(pitch/4)*(i+y)) = *(img+j+(sprite->iPitch/4)*i);
			}
		}
	}

	return true;
}

bool SpriteList::drawMap(int num, BYTE *dib, int x, int y, int width, int height, int pitch, int len)
{
	if (num >= ListSize)
		return false;

	if (s_list[num].bypImage == NULL)
		return false;

	Sprite *sprite = &s_list[num];

	int spWidth = sprite->iWidth*len / 100;
	int spHeight = sprite->iHeight;

	DWORD *dest = (DWORD *)dib;
	DWORD *img = (DWORD *)sprite->bypImage;

	x -= sprite->iCenterPointX;
	y -= sprite->iCenterPointY;

	if (y < 0)
	{
		spHeight += y;
		img = (DWORD *)(sprite->bypImage + sprite->iPitch*(-y));
		y = 0;
	}

	if (y + sprite->iHeight >= height)
	{
		spHeight -= y + sprite->iHeight - height;
	}

	if (x < 0)
	{
		spWidth += x;
		img -= x;
		x = 0;
	}

	if (x + sprite->iWidth >= width)
	{
		spWidth -= x + sprite->iWidth - width;
	}

	if (spWidth <= 0 || spHeight <= 0)
		return false;

	for (int i = 0; i < spHeight; i++)
	{
		for (int j = 0; j < spWidth; j++)
		{
			if ((*(img + j + (sprite->iPitch / 4)*i) & 0x00ffffff) != 0x00ffffff)
			{
				*(dest + j + x + (pitch / 4)*(i + y)) = *(img + j + (sprite->iPitch / 4)*i);
			}
			else
			{
				*(dest + j + x + (pitch / 4)*(i + y)) = 0x00ffffff;
			}
		}
	}

	return true;
}

bool SpriteList::drawColor(int num, BYTE *dib, int x, int y, int width, int height, int pitch, int color, int len)
{
	if (num >= ListSize)
		return false;

	if (s_list[num].bypImage == NULL)
		return false;

	Sprite *sprite = &s_list[num];

	int spWidth = sprite->iWidth*len / 100;
	int spHeight = sprite->iHeight;

	DWORD *dest = (DWORD *)dib;
	DWORD *img = (DWORD *)sprite->bypImage;

	x -= sprite->iCenterPointX;
	y -= sprite->iCenterPointY;

	if (y < 0)
	{
		spHeight += y;
		img = (DWORD *)(sprite->bypImage + sprite->iPitch*(-y));
		y = 0;
	}

	if (y + sprite->iHeight >= height)
	{
		spHeight -= y + sprite->iHeight - height;
	}

	if (x < 0)
	{
		spWidth += x;
		img -= x;
		x = 0;
	}

	if (x + sprite->iWidth >= width)
	{
		spWidth -= x + sprite->iWidth - width;
	}

	if (spWidth <= 0 || spHeight <= 0)
		return false;

	for (int i = 0; i < spHeight; i++)
	{
		for (int j = 0; j < spWidth; j++)
		{
			if ((*(img + j + (sprite->iPitch / 4)*i) & 0x00ffffff) != 0x00ffffff)
			{
				*(dest + j + x + (pitch / 4)*(i + y)) = *(img + j + (sprite->iPitch / 4)*i)|color;
			}
			int temp;
		}
	}

	return true;
}