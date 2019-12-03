#include "Frame.h"

static int test;

Frame::Frame(int max)
	:maxFrame(max), FrameTime(1000/max),TickTime(0)
{
	BeforeTime=timeGetTime();
	//test = BeforeTime;
}

bool Frame::FrameSkip()
{
	NowTime = timeGetTime();
	TickTime += NowTime - BeforeTime;


	if (TickTime < FrameTime)//draw
	{
		Sleep(FrameTime - TickTime);
		TickTime = timeGetTime()-NowTime;
	}
	else if(TickTime-FrameTime>=FrameTime)//skip
	{
		TickTime -= FrameTime;
		BeforeTime = timeGetTime();
		return false;
	}
	TickTime -= FrameTime;
	BeforeTime = timeGetTime();

	return true;
	
}