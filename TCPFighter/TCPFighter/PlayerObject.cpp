#include "PlayerObject.h"

PlayerObject::PlayerObject(int Id, int  Type, int x, int y, bool player)
	:BaseObject(Id,Type,x,y),PlayerCharacter(player),HP(100)
{
	ActionCur = STAND;
	ActionOld = STAND;
	DirCur = LEFT;
	DirOld = LEFT;
	SpriteNow = STAND_L_01;
	SpriteStart = STAND_L_01;
	SpriteEnd = STAND_L_03;
	FrameDelay = dfDELAY_STAND;
	DelayCount = 0;
}

bool PlayerObject::ActionProc()
{

	//if (actionInput==-1)
	//	return false;
	//
	if (actionInput == ActionCur)
		return false;
	
	switch (actionInput)
	{
	case STAND:
		SetActionStand();
		//SetSprite();
		break;
	case MOVE:
		SetActionMove();
		//SetSprite();
		break;
	case ATTACK1:
		SetActionAttack1();
		break;
	case ATTACK2:
		SetActionAttack2();
		break;
	case ATTACK3:
		SetActionAttack3();
		break;
	}
	return true;
}
int PlayerObject::GetDirection() const
{
	
	return DirCur;
}
char PlayerObject::GetHP() const
{
	return HP;
}
bool PlayerObject::InputActionProc()
{
	return true;
}
bool PlayerObject::IsPlayer() const
{
	return PlayerCharacter;
}
bool PlayerObject::Render(SpriteList &s, ScreenDib &dib)
{	
	s.draw(SpriteNow, dib.GetDibBuffer(),curX, curY, dib.GetWidth(), dib.GetHeight(), dib.GetPitch());
	
	return true;
}
bool PlayerObject::Run()
{
	NextFrame();
	ActionProc();

	char up=0;
	char right=0;
	if (ActionCur == MOVE)
	{
		switch (DirCur)
		{
		case dfPACKET_MOVE_DIR_LL:
			right=-1;
			break;
		case dfPACKET_MOVE_DIR_LU:
			right = -1;
			up = -1;
			break;
		case dfPACKET_MOVE_DIR_LD:
			right = -1;
			up = 1;
			break;
		case dfPACKET_MOVE_DIR_RR:
			right = 1;
			break;
		case dfPACKET_MOVE_DIR_RU:
			right = 1;
			up = -1;
			break;
		case dfPACKET_MOVE_DIR_RD:
			right = 1;
			up = 1;
			break;
		case dfPACKET_MOVE_DIR_DD:
			up = 1;
			break;
		case dfPACKET_MOVE_DIR_UU:
			up = -1;
			break;
		}

		if (up > 0&&curY<dfRANGE_MOVE_BOTTOM)
		{
			curY+=2;
		}
		else if (up < 0 && curY > dfRANGE_MOVE_TOP)
		{
			curY-=2;
		}

		if (right > 0 && curX < dfRANGE_MOVE_RIGHT)
		{
			curX+=3;
		}
		else if (right < 0 && curX > dfRANGE_MOVE_LEFT)
		{
			curX-=3;
		}

	}
	//curX++;
	return true;
}
bool PlayerObject::SetActionAttack1()
{
	ActionOld = ActionCur;
	//actionInput = -1;
	FrameDelay = dfDELAY_ATTACK1;
	return true;
}
bool PlayerObject::SetActionAttack2()
{
	ActionOld = ActionCur;
	//actionInput = -1;
	FrameDelay = dfDELAY_ATTACK2;
	return true;
}
bool PlayerObject::SetActionAttack3()
{
	ActionOld = ActionCur;
	//actionInput = -1;
	FrameDelay = dfDELAY_ATTACK3;
	return true;
}
bool PlayerObject::SetActionMove()
{
	ActionOld = ActionCur;
	ActionCur = MOVE;
	//actionInput = -1;
	FrameDelay = dfDELAY_MOVE;
	return true;
}
bool PlayerObject::SetActionStand()
{
	if (ActionCur != STAND)
	{
		//Set
	}

	//actionInput = -1;

	ActionOld = ActionCur;
	ActionCur = STAND;
	FrameDelay = dfDELAY_STAND;
	return true;
}
void PlayerObject::SetDirection(int dir)
{
	DirOld = DirCur;
	DirCur = dir;
	
}
void PlayerObject::SetHP(char newHP)
{
	HP = newHP;
}

