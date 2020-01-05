#include "network.h"

int main()
{
	initNetwork();

	while (1)
	{
		networkProcess();
	}

	return 0;
}