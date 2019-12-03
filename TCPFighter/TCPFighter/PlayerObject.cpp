#include "PlayerObject.h"
#include "PacketDefine.h"
#include "network.h"

PlayerObject::PlayerObject(int Id, int  Type, int direction, int x, int y,char hp, bool player)
	:BaseObject(Id,Type,x,y),PlayerCharacter(player),HP(hp),hit(false)
{
	ActionCur = STAND;
	ActionOld = STAND;
	DirCur = direction;
	DirOld = direction;

	if (DirCur == LEFT)
	{
		SpriteStart = STAND_L_01;
		SpriteNow = STAND_L_01;
		SpriteEnd = STAND_L_05;
	}
	else if (DirCur == RIGHT)
	{
		SpriteStart = STAND_R_01;
		SpriteNow = STAND_R_01;
		SpriteEnd = STAND_R_05;
	}
	FrameDelay = dfDELAY_STAND;
	DelayCount = 0;
}

bool PlayerObject::ActionProc()
{

	//if (actionInput==-1)
	//	return false;
	//
	

	if (ActionCur >= ATTACK1 && ActionCur <= ATTACK3)
	{
		if (!isEndFrame())
		{
			return false;
		}
		else if (actionInput == ActionCur)
		{
			ActionCur = STAND;
		}
		else
		{
			SetActionStand();
		}
	}
	else if (PlayerCharacter)
	{
		if (actionInput == ActionCur)
			return false;
	}
	
	switch (actionInput)
	{
	case STAND:
		SetActionStand();
		//SetSprite();
		break;
	case MOVE_LL:
	case MOVE_LD:
	case MOVE_LU:
		SetDirection(LEFT);
		SetActionMove(actionInput);
		break;
	case MOVE_RR:
	case MOVE_RU:
	case MOVE_RD:
		SetDirection(RIGHT);
		SetActionMove(actionInput);
		break;
	case MOVE_UU:
	case MOVE_DD:
		SetActionMove(actionInput);
		//SetActionMove();
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
	s.draw(SHADOW, dib.GetDibBuffer(), curX, curY, dib.GetWidth(), dib.GetHeight(), dib.GetPitch());
	if (PlayerCharacter)
	{
		s.drawColor(SpriteNow, dib.GetDibBuffer(), curX, curY, dib.GetWidth(), dib.GetHeight(), dib.GetPitch(), 0x000000ff);
	}
	else
		s.draw(SpriteNow, dib.GetDibBuffer(),curX, curY, dib.GetWidth(), dib.GetHeight(), dib.GetPitch());
	s.draw(HPGUAGE, dib.GetDibBuffer(), curX-35, curY+10, dib.GetWidth(), dib.GetHeight(), dib.GetPitch(),GetHP());
	
	return true;
} 
bool PlayerObject::Run()
{
	NextFrame();
	ActionProc();

	

	switch (ActionCur)
	{
	case STAND:
		break;
	case MOVE_LL:
		movePosition(-MOVE_SPEED_X, 0);
		break;
	case MOVE_LD:
		movePosition(-MOVE_SPEED_X, MOVE_SPEED_Y);
		break;
	case MOVE_LU:
		movePosition(-MOVE_SPEED_X, -MOVE_SPEED_Y);
		break;
	case MOVE_RR:
		movePosition(MOVE_SPEED_X, 0);
		break;
	case MOVE_RU:
		movePosition(MOVE_SPEED_X, -MOVE_SPEED_Y);
		break;
	case MOVE_RD:
		movePosition(MOVE_SPEED_X, MOVE_SPEED_Y);
		break;
	case MOVE_UU:
		movePosition(0, -MOVE_SPEED_Y);
		break;
	case MOVE_DD:
		movePosition(0, MOVE_SPEED_Y);
		break;
	case ATTACK1:
		break;
	case ATTACK2:
		break;
	case ATTACK3:
		break;
	}
	if (hit && HitCount >= HitDelay)
	{
		hit = false;

		return false;
	}

	return true;
}
bool PlayerObject::SetActionAttack1()
{
	if (ActionCur == ATTACK1)
		return false;
	ActionOld = ActionCur;
	actionInput = -1;
	FrameDelay = dfDELAY_ATTACK1;
	ActionCur = ATTACK1;
	if (DirCur == LEFT)
	{
		SpriteStart = ATTACK1_L_01;
		SpriteNow = ATTACK1_L_01;
		SpriteEnd = ATTACK1_L_04;
	}
	else if (DirCur == RIGHT)
	{
		SpriteStart = ATTACK1_R_01;
		SpriteNow = ATTACK1_R_01;
		SpriteEnd = ATTACK1_R_04;
	}
	DelayCount = 0;
	EndFrame = false;

	if (PlayerCharacter&&ActionCur != ActionOld)
	{
		if (ActionOld >= MOVE_LL && ActionOld <= MOVE_LD)
		{
			st_NETWORK_PACKET_HEADER pHeader;
			pHeader.byCode = dfNETWORK_PACKET_CODE;
			pHeader.bySize = sizeof(stPACKET_CS_MOVE_STOP);
			pHeader.byType = dfPACKET_CS_MOVE_STOP;
			pHeader.byTemp = NULL;
			stPACKET_CS_MOVE_STOP payload;
			payload.Direction = DirCur;
			payload.X = curX;
			payload.Y = curY;

			SendPacket(&pHeader, (char *)&payload);
		}

		st_NETWORK_PACKET_HEADER pHeader;
		pHeader.byCode = dfNETWORK_PACKET_CODE;
		pHeader.bySize = sizeof(stPACKET_CS_ATTACK);
		pHeader.byType = dfPACKET_CS_ATTACK1;
		pHeader.byTemp = NULL;
		stPACKET_CS_MOVE_STOP payload;
		payload.Direction = DirCur;
		payload.X = curX;
		payload.Y = curY;

		SendPacket(&pHeader, (char *)&payload);
	}

	return true;
}
bool PlayerObject::SetActionAttack2()
{
	if (ActionCur == ATTACK2)
		return false;
	ActionOld = ActionCur;
	actionInput = -1;
	FrameDelay = dfDELAY_ATTACK2;
	ActionCur = ATTACK2;

	if (DirCur == LEFT)
	{
		SpriteStart = ATTACK2_L_01;
		SpriteNow = ATTACK2_L_01;
		SpriteEnd = ATTACK2_L_04;
	}
	else if (DirCur == RIGHT)
	{
		SpriteStart = ATTACK2_R_01;
		SpriteNow = ATTACK2_R_01;
		SpriteEnd = ATTACK2_R_04;
	}
	DelayCount = 0;
	EndFrame = false;

	if (PlayerCharacter&&ActionCur != ActionOld)
	{
		if (ActionOld >= MOVE_LL && ActionOld <= MOVE_LD)
		{
			st_NETWORK_PACKET_HEADER pHeader;
			pHeader.byCode = dfNETWORK_PACKET_CODE;
			pHeader.bySize = sizeof(stPACKET_CS_MOVE_STOP);
			pHeader.byType = dfPACKET_CS_MOVE_STOP;
			pHeader.byTemp = NULL;
			stPACKET_CS_MOVE_STOP payload;
			payload.Direction = DirCur;
			payload.X = curX;
			payload.Y = curY;

			SendPacket(&pHeader, (char *)&payload);
		}

		st_NETWORK_PACKET_HEADER pHeader;
		pHeader.byCode = dfNETWORK_PACKET_CODE;
		pHeader.bySize = sizeof(stPACKET_CS_ATTACK);
		pHeader.byType = dfPACKET_CS_ATTACK2;
		pHeader.byTemp = NULL;
		stPACKET_CS_MOVE_STOP payload;
		payload.Direction = DirCur;
		payload.X = curX;
		payload.Y = curY;

		SendPacket(&pHeader, (char *)&payload);
	}

	return true;
}
bool PlayerObject::SetActionAttack3()
{
	if (ActionCur == ATTACK3)
		return false;
	ActionOld = ActionCur;
	actionInput = -1;
	FrameDelay = dfDELAY_ATTACK3;
	ActionCur = ATTACK3;

	if (DirCur == LEFT)
	{
		SpriteStart = ATTACK3_L_01;
		SpriteNow = ATTACK3_L_01;
		SpriteEnd = ATTACK3_L_06;
	}
	else if (DirCur == RIGHT)
	{
		SpriteStart = ATTACK3_R_01;
		SpriteNow = ATTACK3_R_01;
		SpriteEnd = ATTACK3_R_06;
	}
	DelayCount = 0;
	EndFrame = false;

	if (PlayerCharacter&&ActionCur != ActionOld)
	{
		if (ActionOld >= MOVE_LL && ActionOld <= MOVE_LD)
		{
			st_NETWORK_PACKET_HEADER pHeader;
			pHeader.byCode = dfNETWORK_PACKET_CODE;
			pHeader.bySize = sizeof(stPACKET_CS_MOVE_STOP);
			pHeader.byType = dfPACKET_CS_MOVE_STOP;
			pHeader.byTemp = NULL;
			stPACKET_CS_MOVE_STOP payload;
			payload.Direction = DirCur;
			payload.X = curX;
			payload.Y = curY;

			SendPacket(&pHeader, (char *)&payload);
		}

		st_NETWORK_PACKET_HEADER pHeader;
		pHeader.byCode = dfNETWORK_PACKET_CODE;
		pHeader.bySize = sizeof(stPACKET_CS_ATTACK);
		pHeader.byType = dfPACKET_CS_ATTACK3;
		pHeader.byTemp = NULL;
		stPACKET_CS_MOVE_STOP payload;
		payload.Direction = DirCur;
		payload.X = curX;
		payload.Y = curY;

		SendPacket(&pHeader, (char *)&payload);
	}

	return true;
}
bool PlayerObject::SetActionMove(int dir)
{
	ActionOld = ActionCur;
	ActionCur = dir;
	actionInput = -1;
	FrameDelay = dfDELAY_MOVE;

	if (PlayerCharacter&&ActionCur != ActionOld)
	{
		st_NETWORK_PACKET_HEADER pHeader;
		pHeader.byCode = dfNETWORK_PACKET_CODE;
		pHeader.bySize = sizeof(stPACKET_CS_MOVE_START);
		pHeader.byType = dfPACKET_CS_MOVE_START;
		pHeader.byTemp = NULL;
		stPACKET_CS_MOVE_START payload;
		payload.Direction = ActionCur;
		payload.X = curX;
		payload.Y = curY;

		SendPacket(&pHeader, (char *)&payload);
	}

	if (DirCur == DirOld&&(ActionOld>=MOVE_LL&&ActionOld<=MOVE_DD))
		return true;

	if (DirCur == LEFT)
	{
		SpriteStart = MOVE_L_01;
		SpriteNow = MOVE_L_01;
		SpriteEnd = MOVE_L_12;
	}
	else if (DirCur == RIGHT)
	{
		SpriteStart = MOVE_R_01;
		SpriteNow = MOVE_R_01;
		SpriteEnd = MOVE_R_12;
	}
	DelayCount = 0;
	

	return true;
}
bool PlayerObject::SetActionStand()
{
	//actionInput = -1;

	ActionOld = ActionCur;
	ActionCur = STAND;
	FrameDelay = dfDELAY_STAND;

	if (DirCur == LEFT)
	{
		SpriteStart = STAND_L_01;
		SpriteNow = STAND_L_01;
		SpriteEnd = STAND_L_05;
	}
	else if (DirCur == RIGHT)
	{
		SpriteStart = STAND_R_01;
		SpriteNow = STAND_R_01;
		SpriteEnd = STAND_R_05;
	}
	DelayCount = 0;

	if (PlayerCharacter&&ActionCur != ActionOld&&!(ActionOld>=ATTACK1&&ActionOld<=ATTACK3))
	{
		st_NETWORK_PACKET_HEADER pHeader;
		pHeader.byCode = dfNETWORK_PACKET_CODE;
		pHeader.bySize = sizeof(stPACKET_CS_MOVE_STOP);
		pHeader.byType = dfPACKET_CS_MOVE_STOP;
		pHeader.byTemp = NULL;
		stPACKET_CS_MOVE_STOP payload;
		payload.Direction = DirCur;
		payload.X = curX;
		payload.Y = curY;

		SendPacket(&pHeader, (char *)&payload);
	}

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

void PlayerObject::movePosition(int x, int y)
{
	if (!(curX + x >= dfRANGE_MOVE_LEFT && curX + x <= dfRANGE_MOVE_RIGHT))
	{
		return;
	}

	if (!(curY + y >= dfRANGE_MOVE_TOP && curY + y <= dfRANGE_MOVE_BOTTOM))
	{
		return;
	}

	curX += x;
	curY += y;
}

void PlayerObject::SetPosition(int x, int y)
{
	curX = x;
	curY = y;
}

DWORD PlayerObject::GetAction() const
{
	if(actionInput==-1)
		return ActionCur;
	
	return actionInput;
}