#include <iostream>
#include "network.h"

#define PORT 6000

int main()
{
	initNetwork(PORT);

	while (1)
	{
		networkProcess();
	}
}