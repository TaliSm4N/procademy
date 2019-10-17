#include <iostream>
#include <cstdlib>
#include <ctime>
#include <Windows.h>
#include "Screen.h"
#include "SceneManager.h"

#define FRAME 100

void init();

Screen *buffer;
SceneManager *sManager;

int main()
{
	init();

	while (1)
	{
		buffer->clear();
		sManager->run();
		buffer->flip();
		Sleep(FRAME);
		
	}
}

void init()
{
	buffer = Screen::GetInstance();
	sManager = SceneManager::GetInstance();
	srand(time(NULL));
}