#pragma once
#include <Windows.h>

class Frame
{
public:
	Frame(int max);

	bool FrameSkip();
private:
	int maxFrame;
	int FrameTime;
	DWORD TickTime;
	DWORD BeforeTime;
	DWORD NowTime;
};