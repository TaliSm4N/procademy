#include "Game.h"
#include "ScreenDib.h"
#include "Sprite.h"
#include "Frame.h"
#include <Windows.h>
#include <list>

#include "BaseObject.h"
#include "PlayerObject.h"



ScreenDib g_screen(640, 480, 32);


SpriteList s(100);

std::list<BaseObject *> objectList;

PlayerObject *myPlayer=NULL;

BOOL init()
{
	if (!s.settingSprite(MAP, (char *)"SpriteData\\_Map.bmp", 0, 0))
		return false;

	if (!s.settingSprite(ATTACK1_L_01, (char *)"SpriteData\\Attack1_L_01.bmp", 71, 90))
		return false;
	if (!s.settingSprite(ATTACK1_L_02, (char *)"SpriteData\\Attack1_L_02.bmp", 71, 90))
		return false;
	if (!s.settingSprite(ATTACK1_L_03, (char *)"SpriteData\\Attack1_L_03.bmp", 71, 90))
		return false;
	if (!s.settingSprite(ATTACK1_L_04, (char *)"SpriteData\\Attack1_L_04.bmp", 71, 90))
		return false;

	if (!s.settingSprite(ATTACK1_R_01, (char *)"SpriteData\\Attack1_R_01.bmp", 71, 90))
		return false;
	if (!s.settingSprite(ATTACK1_R_02, (char *)"SpriteData\\Attack1_R_02.bmp", 71, 90))
		return false;
	if (!s.settingSprite(ATTACK1_R_03, (char *)"SpriteData\\Attack1_R_03.bmp", 71, 90))
		return false;
	if (!s.settingSprite(ATTACK1_R_04, (char *)"SpriteData\\Attack1_R_04.bmp", 71, 90))
		return false;

	if (!s.settingSprite(ATTACK2_L_01, (char *)"SpriteData\\Attack2_L_01.bmp", 71, 90) )
		return false;														 
	if (!s.settingSprite(ATTACK2_L_02, (char *)"SpriteData\\Attack2_L_02.bmp", 71, 90) )
		return false;														 
	if (!s.settingSprite(ATTACK2_L_03, (char *)"SpriteData\\Attack2_L_03.bmp", 71, 90) )
		return false;														 
	if (!s.settingSprite(ATTACK2_L_04, (char *)"SpriteData\\Attack2_L_04.bmp", 71, 90) )
		return false;														 
																			 
	if (!s.settingSprite(ATTACK2_R_01, (char *)"SpriteData\\Attack2_R_01.bmp", 71, 90) )
		return false;														 
	if (!s.settingSprite(ATTACK2_R_02, (char *)"SpriteData\\Attack2_R_02.bmp", 71, 90) )
		return false;														 
	if (!s.settingSprite(ATTACK2_R_03, (char *)"SpriteData\\Attack2_R_03.bmp", 71, 90) )
		return false;														 
	if (!s.settingSprite(ATTACK2_R_04, (char *)"SpriteData\\Attack2_R_04.bmp", 71, 90) )
		return false;														 
																			 
	if (!s.settingSprite(ATTACK3_L_01, (char *)"SpriteData\\Attack3_L_01.bmp", 71, 90) )
		return false;
	if (!s.settingSprite(ATTACK3_L_02, (char *)"SpriteData\\Attack3_L_02.bmp", 71, 90) )
		return false;												 
	if (!s.settingSprite(ATTACK3_L_03, (char *)"SpriteData\\Attack3_L_03.bmp", 71, 90) )
		return false;												 
	if (!s.settingSprite(ATTACK3_L_04, (char *)"SpriteData\\Attack3_L_04.bmp", 71, 90) )
		return false;												 
	if (!s.settingSprite(ATTACK3_L_05, (char *)"SpriteData\\Attack3_L_05.bmp", 71, 90) )
		return false;												 
	if (!s.settingSprite(ATTACK3_L_06, (char *)"SpriteData\\Attack3_L_06.bmp", 71, 90) )
		return false;												 
																	 
	if (!s.settingSprite(ATTACK3_R_01, (char *)"SpriteData\\Attack3_R_01.bmp", 71, 90))
		return false;
	if (!s.settingSprite(ATTACK3_R_02, (char *)"SpriteData\\Attack3_R_02.bmp", 71, 90))
		return false;
	if (!s.settingSprite(ATTACK3_R_03, (char *)"SpriteData\\Attack3_R_03.bmp", 71, 90))
		return false;
	if (!s.settingSprite(ATTACK3_R_04, (char *)"SpriteData\\Attack3_R_04.bmp", 71, 90))
		return false;
	if (!s.settingSprite(ATTACK3_R_05, (char *)"SpriteData\\Attack3_R_05.bmp", 71, 90))
		return false;
	if (!s.settingSprite(ATTACK3_R_06, (char *)"SpriteData\\Attack3_R_06.bmp", 71, 90))
		return false;

	if (!s.settingSprite(HPGUAGE, (char *)"SpriteData\\HPGuage.bmp", 0, 0))
		return false;

	if (!s.settingSprite(MOVE_L_01, (char *)"SpriteData\\Move_L_01.bmp", 71, 90) )
		return false;											  
	if (!s.settingSprite(MOVE_L_02, (char *)"SpriteData\\Move_L_02.bmp", 71, 90) )
		return false;											  
	if (!s.settingSprite(MOVE_L_03, (char *)"SpriteData\\Move_L_03.bmp", 71, 90) )
		return false;											  
	if (!s.settingSprite(MOVE_L_04, (char *)"SpriteData\\Move_L_04.bmp", 71, 90) )
		return false;											  
	if (!s.settingSprite(MOVE_L_05, (char *)"SpriteData\\Move_L_05.bmp", 71, 90) )
		return false;											  
	if (!s.settingSprite(MOVE_L_06, (char *)"SpriteData\\Move_L_06.bmp", 71, 90) )
		return false;											  
	if (!s.settingSprite(MOVE_L_07, (char *)"SpriteData\\Move_L_07.bmp", 71, 90) )
		return false;											  
	if (!s.settingSprite(MOVE_L_08, (char *)"SpriteData\\Move_L_08.bmp", 71, 90) )
		return false;											  
	if (!s.settingSprite(MOVE_L_09, (char *)"SpriteData\\Move_L_09.bmp", 71, 90) )
		return false;											  
	if (!s.settingSprite(MOVE_L_10, (char *)"SpriteData\\Move_L_10.bmp", 71, 90) )
		return false;											  
	if (!s.settingSprite(MOVE_L_11, (char *)"SpriteData\\Move_L_11.bmp", 71, 90) )
		return false;											  
	if (!s.settingSprite(MOVE_L_12, (char *)"SpriteData\\Move_L_12.bmp", 71, 90) )
		return false;											  
																  
	if (!s.settingSprite(MOVE_R_01, (char *)"SpriteData\\Move_R_01.bmp", 71, 90))
		return false;
	if (!s.settingSprite(MOVE_R_02, (char *)"SpriteData\\Move_R_02.bmp", 71, 90))
		return false;
	if (!s.settingSprite(MOVE_R_03, (char *)"SpriteData\\Move_R_03.bmp", 71, 90))
		return false;
	if (!s.settingSprite(MOVE_R_04, (char *)"SpriteData\\Move_R_04.bmp", 71, 90))
		return false;
	if (!s.settingSprite(MOVE_R_05, (char *)"SpriteData\\Move_R_05.bmp", 71, 90))
		return false;
	if (!s.settingSprite(MOVE_R_06, (char *)"SpriteData\\Move_R_06.bmp", 71, 90))
		return false;
	if (!s.settingSprite(MOVE_R_07, (char *)"SpriteData\\Move_R_07.bmp", 71, 90))
		return false;
	if (!s.settingSprite(MOVE_R_08, (char *)"SpriteData\\Move_R_08.bmp", 71, 90))
		return false;
	if (!s.settingSprite(MOVE_R_09, (char *)"SpriteData\\Move_R_09.bmp", 71, 90))
		return false;
	if (!s.settingSprite(MOVE_R_10, (char *)"SpriteData\\Move_R_10.bmp", 71, 90))
		return false;
	if (!s.settingSprite(MOVE_R_11, (char *)"SpriteData\\Move_R_11.bmp", 71, 90))
		return false;
	if (!s.settingSprite(MOVE_R_12, (char *)"SpriteData\\Move_R_12.bmp", 71, 90))
		return false;

	if (!s.settingSprite(SHADOW, (char *)"SpriteData\\Shadow.bmp", 32, 4))
		return false;

	if (!s.settingSprite(STAND_L_01, (char *)"SpriteData\\Stand_L_01.bmp", 71, 90) )
		return false;											   
	if (!s.settingSprite(STAND_L_02, (char *)"SpriteData\\Stand_L_02.bmp", 71, 90) )
		return false;											   
	if (!s.settingSprite(STAND_L_03, (char *)"SpriteData\\Stand_L_03.bmp", 71, 90) )
		return false;											   
																   
	if (!s.settingSprite(STAND_R_01, (char *)"SpriteData\\Stand_R_01.bmp", 71, 90))
		return false;
	if (!s.settingSprite(STAND_R_02, (char *)"SpriteData\\Stand_R_02.bmp", 71, 90))
		return false;
	if (!s.settingSprite(STAND_R_03, (char *)"SpriteData\\Stand_R_03.bmp", 71, 90))
		return false;

	if (!s.settingSprite(XSPARK_01, (char *)"SpriteData\\xSpark_1.bmp", 70, 70))
		return false;
	if (!s.settingSprite(XSPARK_02, (char *)"SpriteData\\xSpark_2.bmp", 70, 70))
		return false;
	if (!s.settingSprite(XSPARK_03, (char *)"SpriteData\\xSpark_3.bmp", 70, 70))
		return false;
	if (!s.settingSprite(XSPARK_04, (char *)"SpriteData\\xSpark_4.bmp", 70, 70))
		return false;

	PlayerObject *testP = new PlayerObject(0, PLAYER,320,240,true);
	myPlayer = testP;
	objectList.push_back(testP);

	return true;
}

BOOL keyboard()
{
	char ud = 0;
	char rl = 0;

	if (myPlayer == NULL)
		return false;

	if (GetAsyncKeyState(VK_UP))
	{
		ud++;
	}
	if (GetAsyncKeyState(VK_DOWN))
	{
		ud--;
	}
	if (GetAsyncKeyState(VK_LEFT))
	{
		rl--;
	}
	if (GetAsyncKeyState(VK_RIGHT))
	{
		rl++;
	}

	if (ud < 0)
	{
		if (rl < 0)
		{
			myPlayer->SetDirection(dfPACKET_MOVE_DIR_LD);
		}
		else if (rl == 0)
		{
			myPlayer->SetDirection(dfPACKET_MOVE_DIR_DD);
		}
		else if (rl > 0)
		{
			myPlayer->SetDirection(dfPACKET_MOVE_DIR_RD);
		}
	}
	else if(ud == 0)
	{
		if (rl < 0)
		{
			myPlayer->SetDirection(dfPACKET_MOVE_DIR_LL);
		}
		else if (rl == 0)
		{
			myPlayer->ActionInput(STAND);
			return true;
		}
		else if (rl > 0)
		{
			myPlayer->SetDirection(dfPACKET_MOVE_DIR_RR);
		}
	}
	else if (ud > 0)
	{
		if (rl < 0)
		{
			myPlayer->SetDirection(dfPACKET_MOVE_DIR_LU);
		}
		else if (rl == 0)
		{
			myPlayer->SetDirection(dfPACKET_MOVE_DIR_UU);
		}
		else if (rl > 0)
		{
			myPlayer->SetDirection(dfPACKET_MOVE_DIR_RU);
		}
	}
	myPlayer->ActionInput(MOVE);

	return true;
}

BOOL Draw(HWND hWnd)
{
	s.draw(MAP, g_screen.GetDibBuffer(), 0, 0, g_screen.GetWidth(), g_screen.GetHeight(), g_screen.GetPitch());
	
	//s.draw(STAND_L_02, g_screen.GetDibBuffer(), 640, 100, g_screen.GetWidth(), g_screen.GetHeight(), g_screen.GetPitch());
	for (auto iter = objectList.begin(); iter != objectList.end(); iter++)
	{
		(*iter)->Render(s,g_screen);
	}

	g_screen.Flip(hWnd);

	return true;
}

BOOL Run()
{
	for (auto iter = objectList.begin(); iter != objectList.end(); iter++)
	{
		(*iter)->Run();
	}

	return true;
}

int GameFrame(HWND hWnd)
{
	static Frame frame(50);
	
	keyboard();

	Run();
	
	//if(frame.FrameSkip())
	if(frame.FrameSkip())
		Draw(hWnd);
	Sleep(0);
	return 0;
}