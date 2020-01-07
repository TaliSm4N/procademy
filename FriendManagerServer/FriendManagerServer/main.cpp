#include "network.h"
#include <iostream>
int main()
{
	initNetwork();

	DWORD tick;

	while (1)
	{
		//tick = timeGetTime();
		networkProcess();
		//std::cout << "process time:" << timeGetTime() - tick << std::endl;
	}

	return 0;
}