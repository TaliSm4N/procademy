#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>

enum eSprite
{
	MAP = 0,
	ATTACK1_L_01, ATTACK1_L_02, ATTACK1_L_03, ATTACK1_L_04,
	ATTACK1_R_01, ATTACK1_R_02, ATTACK1_R_03, ATTACK1_R_04,
	ATTACK2_L_01, ATTACK2_L_02, ATTACK2_L_03, ATTACK2_L_04,
	ATTACK2_R_01, ATTACK2_R_02, ATTACK2_R_03, ATTACK2_R_04,
	ATTACK3_L_01, ATTACK3_L_02, ATTACK3_L_03, ATTACK3_L_04, ATTACK3_L_05, ATTACK3_L_06,
	ATTACK3_R_01, ATTACK3_R_02, ATTACK3_R_03, ATTACK3_R_04, ATTACK3_R_05, ATTACK3_R_06,
	HPGUAGE,
	MOVE_L_01, MOVE_L_02, MOVE_L_03, MOVE_L_04, MOVE_L_05, MOVE_L_06, MOVE_L_07, MOVE_L_08, MOVE_L_09, MOVE_L_10, MOVE_L_11, MOVE_L_12,
	MOVE_R_01, MOVE_R_02, MOVE_R_03, MOVE_R_04, MOVE_R_05, MOVE_R_06, MOVE_R_07, MOVE_R_08, MOVE_R_09, MOVE_R_10, MOVE_R_11, MOVE_R_12,
	SHADOW,
	STAND_L_01, STAND_L_02, STAND_L_03, STAND_L_04, STAND_L_05,
	STAND_R_01, STAND_R_02, STAND_R_03, STAND_R_04, STAND_R_05,
	XSPARK_01, XSPARK_02, XSPARK_03, XSPARK_04
};

class SpriteList
{
public:
	SpriteList(int size=10);
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
	bool settingSprite(int num, const char *path, int cX, int cY);
	void releaseSprite(int num);
	bool draw(int num, BYTE *dib, int x, int y,int width,int height,int pitch,int len=100);
	bool drawColor(int num, BYTE *dib, int x, int y, int width, int height, int pitch,int color, int len = 100);
private:
	int ListSize;
	Sprite *s_list;
};