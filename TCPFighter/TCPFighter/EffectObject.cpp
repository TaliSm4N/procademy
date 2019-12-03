#include "EffectObject.h"
/*
class EffectObject : public BaseObject
{
public:
	virtual bool Render(SpriteList &s, ScreenDib &dib);
	virtual bool Run();
	EffectObject(int Id, int  Type, int x, int y);
private:
	bool effectStart;
	DWORD AttackID
};
*/

EffectObject::EffectObject(int del, int  Type, int x, int y)
	:BaseObject(-1,Type,x,y),delay(del),count(0)
{
	effectStart = true;
	SpriteStart = XSPARK_01;
	SpriteNow = XSPARK_01;
	SpriteEnd = XSPARK_04;

	FrameDelay = dfDELAY_EFFECT;
	EndFrame = false;
	DelayCount = 0;
}

bool EffectObject::Run()
{
	if (count >= delay)
		NextFrame();
	else
		count++;

	if (isEndFrame())
	{
		return false;
	}
	return true;
}

bool EffectObject::Render(SpriteList &s, ScreenDib &dib)
{
	if (count >= delay)
		s.draw(SpriteNow, dib.GetDibBuffer(), curX, curY, dib.GetWidth(), dib.GetHeight(), dib.GetPitch());

	return true;
}