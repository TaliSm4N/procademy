#include "Game.h"
#include "ScreenDib.h"
#include "Sprite.h"
#include "Frame.h"
#include "EffectObject.h"
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
	if (!s.settingSprite(STAND_L_04, (char *)"SpriteData\\Stand_L_02.bmp", 71, 90))
		return false;
	if (!s.settingSprite(STAND_L_05, (char *)"SpriteData\\Stand_L_01.bmp", 71, 90))
		return false;
																   
	if (!s.settingSprite(STAND_R_01, (char *)"SpriteData\\Stand_R_01.bmp", 71, 90))
		return false;
	if (!s.settingSprite(STAND_R_02, (char *)"SpriteData\\Stand_R_02.bmp", 71, 90))
		return false;
	if (!s.settingSprite(STAND_R_03, (char *)"SpriteData\\Stand_R_03.bmp", 71, 90))
		return false;
	if (!s.settingSprite(STAND_R_04, (char *)"SpriteData\\Stand_R_02.bmp", 71, 90))
		return false;
	if (!s.settingSprite(STAND_R_05, (char *)"SpriteData\\Stand_R_01.bmp", 71, 90))
		return false;

	if (!s.settingSprite(XSPARK_01, (char *)"SpriteData\\xSpark_1.bmp", 70, 70))
		return false;
	if (!s.settingSprite(XSPARK_02, (char *)"SpriteData\\xSpark_2.bmp", 70, 70))
		return false;
	if (!s.settingSprite(XSPARK_03, (char *)"SpriteData\\xSpark_3.bmp", 70, 70))
		return false;
	if (!s.settingSprite(XSPARK_04, (char *)"SpriteData\\xSpark_4.bmp", 70, 70))
		return false;

	if (!s.settingSprite(TILE, (char *)"SpriteData\\Tile_01.bmp", 0, 0))
		return false;

	//PlayerObject *testP = new PlayerObject(0, PLAYER,LEFT,320,240,true);
	//PlayerObject *test2P = new PlayerObject(0, PLAYER, RIGHT, 320, 240, false);
	//EffectObject *terE = new EffectObject(0, EFFECT, 100, 100);
	//myPlayer = testP;
	//objectList.push_back(testP);
	//objectList.push_back(test2P);
	//objectList.push_back(terE);

	return true;
}

BOOL keyboard()
{
	char ud = 0;
	char rl = 0;

	if (myPlayer == NULL)
		return false;

	//공격 입력 우선 입력
	//Z
	if (GetAsyncKeyState(0x5A))
	{
		myPlayer->ActionInput(ATTACK1);
		return true;
	}

	//X
	if (GetAsyncKeyState(0x58))
	{
		myPlayer->ActionInput(ATTACK2);
		return true;
	}

	if (GetAsyncKeyState(0x43))
	{
		myPlayer->ActionInput(ATTACK3);
		return true;
	}

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
			myPlayer->ActionInput(MOVE_LD);
		}
		else if (rl == 0)
		{
			myPlayer->ActionInput(MOVE_DD);
		}
		else if (rl > 0)
		{
			myPlayer->ActionInput(MOVE_RD);
		}
	}
	else if(ud == 0)
	{
		if (rl < 0)
		{
			myPlayer->ActionInput(MOVE_LL);
		}
		else if (rl == 0)
		{
			myPlayer->ActionInput(STAND);
			return true;
		}
		else if (rl > 0)
		{
			myPlayer->ActionInput(MOVE_RR);
		}
	}
	else if (ud > 0)
	{
		if (rl < 0)
		{
			myPlayer->ActionInput(MOVE_LU);
		}
		else if (rl == 0)
		{
			myPlayer->ActionInput(MOVE_UU);
		}
		else if (rl > 0)
		{
			myPlayer->ActionInput(MOVE_RU);
		}
	}
	//myPlayer->ActionInput(MOVE);

	return true;
}

BOOL Draw(HWND hWnd)
{
	//s.draw(MAP, g_screen.GetDibBuffer(), 0, 0, g_screen.GetWidth(), g_screen.GetHeight(), g_screen.GetPitch());

	if (objectList.empty())
		return false;

	
	
	for (int i = 0; i < 640 / 64 + 2; i++)
	{
		for (int j = 0; j < 480 / 64 + 2; j++)
		{
			s.drawMap(TILE, g_screen.GetDibBuffer(), i * 64- g_screen.GetCamera()->GetX() % 64, j * 64 - g_screen.GetCamera()->GetY() % 64, g_screen.GetWidth(), g_screen.GetHeight(), g_screen.GetPitch());
		}
	}

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
	BOOL ret;
	for (auto iter = objectList.begin(); iter != objectList.end();)
	{
		ret=(*iter)->Run();

		if (!ret)
		{
			iter = objectList.erase(iter);
		}
		else
			iter++;
	}

	return true;
}

int GameFrame(HWND hWnd, bool active)
{
	static Frame frame(50);
	
	if(active)
		keyboard();

	Run();
	
	//if(frame.FrameSkip())
	if(frame.FrameSkip())
		Draw(hWnd);
	Sleep(0);
	return 0;
}

BOOL CreatePlayer(int id, int dir, int x, int y, char hp,bool player)
{
	PlayerObject *temp = new PlayerObject(id, PLAYER, dir, x, y,hp, player);

	objectList.push_back(temp);

	if (player)
	{
		myPlayer = temp;
		myPlayer->connectCamera(g_screen.GetCamera());
	}

	return true;
}

BOOL DeletePlayer(int id)
{
	for (auto iter = objectList.begin(); iter != objectList.end();iter++)
	{
		if ((*iter)->GetObjectID() == id)
		{
			iter = objectList.erase(iter);
			return true;
		}
	}

	return false;
}

BOOL MovePlayer(int id, int dir,short x,short y)
{
	for (auto iter = objectList.begin(); iter != objectList.end(); iter++)
	{
		if ((*iter)->GetObjectID() == id)
		{
			((PlayerObject *)(*iter))->SetPosition(x, y);
			((PlayerObject *)(*iter))->ActionInput(dir);
			break;
		}
	}

	return true;
}

BOOL StopPlayer(int id, int dir,short x, short y)
{
	for (auto iter = objectList.begin(); iter != objectList.end(); iter++)
	{
		if ((*iter)->GetObjectID() == id)
		{
			((PlayerObject *)(*iter))->SetPosition(x, y);
			((PlayerObject *)(*iter))->ActionInput(STAND);
			break;
		}
	}

	return true;
}

BOOL Attack1Player(int id, int dir, short x, short y)
{
	for (auto iter = objectList.begin(); iter != objectList.end(); iter++)
	{
		if ((*iter)->GetObjectID() == id)
		{
			((PlayerObject *)(*iter))->SetPosition(x, y);
			((PlayerObject *)(*iter))->ActionInput(ATTACK1);
			break;
		}
	}
	return true;
}

BOOL Attack2Player(int id, int dir, short x, short y)
{
	for (auto iter = objectList.begin(); iter != objectList.end(); iter++)
	{
		if ((*iter)->GetObjectID() == id)
		{
			((PlayerObject *)(*iter))->SetPosition(x, y);
			((PlayerObject *)(*iter))->ActionInput(ATTACK2);
			break;
		}
	}
	return true;
}

BOOL Attack3Player(int id, int dir, short x, short y)
{
	for (auto iter = objectList.begin(); iter != objectList.end(); iter++)
	{
		if ((*iter)->GetObjectID() == id)
		{
			((PlayerObject *)(*iter))->SetPosition(x, y);
			((PlayerObject *)(*iter))->ActionInput(ATTACK3);
			break;
		}
	}
	return true;
}

BOOL DamagePlayer(int AttackID, int DamageID, char DamageHP)
{
	bool breakBool = false;
	PlayerObject *hitP = NULL;
	PlayerObject *atkP = NULL;
	EffectObject *eff=NULL;
	for (auto iter = objectList.begin(); iter != objectList.end(); iter++)
	{
		if ((*iter)->GetObjectID() == DamageID)
		{
			((PlayerObject *)(*iter))->SetHP(DamageHP);
			hitP = (PlayerObject *)(*iter);
			if (!breakBool)
				breakBool = true;
			else
				break;
			//break;
		}

		if ((*iter)->GetObjectID() == AttackID)
		{
			atkP = ((PlayerObject *)(*iter));
			//((PlayerObject *)(*iter))->SetHit(DamageID);
			if (!breakBool)
				breakBool = true;
			else
				break;
			//break;
		}
	}

	if (hitP == NULL || atkP == NULL)
		return false;

	switch (atkP->GetAction())
	{
	case ATTACK1:
		eff = new EffectObject(3, EFFECT, hitP->GetCurX(), hitP->GetCurY() - 45);
		break;
	case ATTACK2:
		eff = new EffectObject(6, EFFECT, hitP->GetCurX(), hitP->GetCurY() - 45);
		break;
	case ATTACK3:
		eff = new EffectObject(12, EFFECT, hitP->GetCurX(), hitP->GetCurY() - 45);
		break;
	default:
		break;
	}
	if(eff!=NULL)
		objectList.push_back(eff);
	
	
	return true;
}

BOOL SyncPlayer(DWORD SyncID, WORD x, WORD y)
{
	for (auto iter = objectList.begin(); iter != objectList.end(); iter++)
	{
		if ((*iter)->GetObjectID() == SyncID)
		{
			((PlayerObject *)(*iter))->SetPosition(x, y);
			break;
		}
	}

	return true;
}