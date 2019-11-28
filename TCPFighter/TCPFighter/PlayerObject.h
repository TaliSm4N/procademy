#pragma once
#include "BaseObject.h"

#include "Sprite.h"

#define dfPACKET_MOVE_DIR_LL					0
#define dfPACKET_MOVE_DIR_LU					1
#define dfPACKET_MOVE_DIR_UU					2
#define dfPACKET_MOVE_DIR_RU					3
#define dfPACKET_MOVE_DIR_RR					4
#define dfPACKET_MOVE_DIR_RD					5
#define dfPACKET_MOVE_DIR_DD					6
#define dfPACKET_MOVE_DIR_LD					7

#define dfRANGE_MOVE_TOP	50
#define dfRANGE_MOVE_LEFT	10
#define dfRANGE_MOVE_RIGHT	630
#define dfRANGE_MOVE_BOTTOM	470

#define dfDELAY_STAND	5
#define dfDELAY_MOVE	4
#define dfDELAY_ATTACK1	3
#define dfDELAY_ATTACK2	4
#define dfDELAY_ATTACK3	4
#define dfDELAY_EFFECT	3

#define RIGHT 0
#define LEFT 1

enum Status {STAND=0,MOVE,ATTACK1, ATTACK2, ATTACK3};
class PlayerObject :public BaseObject
{
public:
	PlayerObject(int ObjectId,int ObjectType, int x = 320, int y = 240,bool player=false);
	bool ActionProc();
	int GetDirection() const;
	char GetHP() const;
	bool InputActionProc();
	bool IsPlayer() const;
	virtual bool Render(SpriteList &s, ScreenDib &dib);
	virtual bool Run();
	bool SetActionAttack1();
	bool SetActionAttack2();
	bool SetActionAttack3();
	bool SetActionMove();
	bool SetActionStand();
	void SetDirection(int dir);
	void SetSprite();
	void SetHP(char newHP);

private:
	bool PlayerCharacter;
	char HP;
	DWORD ActionCur;
	DWORD ActionOld;
	int DirCur;
	int DirOld;
	/*
	bool EndFrame;
	DWORD actionInput;
	int curX;
	int curY;
	int DelayCount;
	int FrameDelay;
	int ObjectID;
	int ObjectType;
	int SpriteEnd;
	int SpriteNow;
	int SpriteStart;
	*/
};