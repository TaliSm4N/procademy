#include "RingBuffer.h"

#include <iostream>
#include <ctime>
#include <Windows.h>

char testStr[82] = "1234567890 abcdefghijklmnopqrstuvwxyz 1234567890 abcdefghijklmnopqrstuvwxyz 12345";


int main()
{
	std::cout << "start"<<std::endl;

	srand(time(NULL));

	RingBuffer *rb = new RingBuffer(40);
	int randNum;
	int len = 81;
	int cur = 0;
	int temp;

	char buf[81*3];

	system("mode con cols=81 lines=25");

	while (1)
	{
		if (len == 0)
		{
			len = 81;
			cur = 0;
		}

		randNum = rand() % len+1;

		temp=rb->Enqueue(testStr + cur, randNum);
		cur += temp;
		len -= temp;

		randNum = rand() % rb->GetUseSize();

		temp = rb->Peek(buf, randNum);
		//rb->MoveFront(temp);
		buf[temp] = '\0';
		std::cout << buf;// << std::endl;
		rb->Dequeue(buf, temp);
		Sleep(10);
	}

	return 0;
}