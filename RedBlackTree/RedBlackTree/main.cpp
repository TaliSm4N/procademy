#include <iostream>
#include "RedBlackTree.h"

int main()
{
	int input;
	RBTree *rb = new RBTree();

	while (1)
	{
		std::cout << "�Է��ض� �ּ���>>";
		std::cin >> input;
		rb->InsertNode(input);

		std::cout << std::endl;
	}
}