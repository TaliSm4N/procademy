#pragma once
#include "BaseObject.h"

#define dfDELAY_EFFECT	3

class EffectObject : public BaseObject
{
public:
	virtual bool Render(SpriteList &s, ScreenDib &dib);
	virtual bool Run();
	EffectObject(int del, int  Type, int x, int y);
private:
	bool effectStart;
	DWORD AttackID;
	int delay;
	int count;
};