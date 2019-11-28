#pragma once
#include <Windows.h>
#include "Sprite.h"
#include "ScreenDib.h"

enum OBJECT_TYPE { PLAYER=0,EFFECT};

class BaseObject
{
public:
	BaseObject(int id,int type,int x = 240, int y = 320);
	bool ActionInput(int stat);
	int GetCurX() const;
	int GetCurY() const;
	int GetObjectID() const;
	int GetObjectType() const;
	int GetSprite() const;
	bool isEndFrame() const;
	int NextFrame();
	virtual bool Render(SpriteList &s,ScreenDib &dib)=0;
	virtual bool Run()=0;
protected:
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
};