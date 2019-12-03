#pragma once
#include "BaseObject.h"

#include "Sprite.h"

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

#define LEFT 0
#define RIGHT 4

#define MOVE_SPEED_X 3
#define MOVE_SPEED_Y 2


enum Status {MOVE_LL=0,MOVE_LU,MOVE_UU,
	MOVE_RU,MOVE_RR,MOVE_RD,
	MOVE_DD,MOVE_LD,STAND,ATTACK1, ATTACK2, ATTACK3};
class PlayerObject :public BaseObject
{
public:
	PlayerObject(int ObjectId,int ObjectType, int direction, int x = 320, int y = 240,char hp=100,bool player=false);
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
	bool SetActionMove(int dir);
	bool SetActionStand();
	void SetDirection(int dir);
	void SetHP(char newHP);
	void SetPosition(int x, int y);
	DWORD GetAction() const;
	//void SetHit(int id);

private:
	bool PlayerCharacter;
	char HP;
	DWORD ActionCur;
	DWORD ActionOld;
	int DirCur;
	int DirOld;

	BOOL hit;
	int HitDelay;
	int HitCount;

	void movePosition(int x, int y);
	
	//void setSprite(int stat);
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