#include "BaseObject.h"

BaseObject::BaseObject(int id, int type, int x, int y)
	:curX(x),curY(y),ObjectID(id),ObjectType(type),SpriteStart(0),SpriteNow(0),SpriteEnd(0)
{

}

bool BaseObject::ActionInput(int stat)
{
	actionInput = stat;
	return true;
}

int BaseObject::GetCurX() const
{
	return curX;
}

int BaseObject::GetCurY() const
{
	return curY;
}

int BaseObject::GetObjectID() const
{
	return ObjectID;
}

int BaseObject::GetObjectType() const
{
	return ObjectType;
}

int BaseObject::GetSprite() const
{
	return SpriteNow;
}

bool BaseObject::isEndFrame() const
{
	return EndFrame;
}
int BaseObject::NextFrame()
{
	DelayCount++;
	if (DelayCount >= FrameDelay)
	{
		DelayCount = 0;
		SpriteNow++;
		if (SpriteNow > SpriteEnd)
		{
			SpriteNow = SpriteStart;
			EndFrame = true;
		}
	}

	return 0;
}