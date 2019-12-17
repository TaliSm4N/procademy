#include <iostream>
#include <Windows.h>

#include "RedBlackTree.h"

int main()
{
	int input;
	RBTree *rb = new RBTree();


	rb->InsertNode(10);
	rb->InsertNode(15);
	rb->InsertNode(23);
	rb->InsertNode(27);
	rb->InsertNode(25);
	rb->InsertNode(13);
	rb->InsertNode(18);
	rb->InsertNode(8);
	rb->InsertNode(5);
	rb->InsertNode(9);
	rb->InsertNode(17);
	rb->InsertNode(33);
	rb->InsertNode(26);
	rb->InsertNode(77);
	rb->InsertNode(50);

	while (1)
	{
		std::cout << "입력해라 애송이>>";
		std::cin >> input;
		rb->DeleteNode(input);

		std::cout << std::endl;
	}

}