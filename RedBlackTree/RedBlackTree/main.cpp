#include <iostream>
#include "RedBlackTree.h"

int main()
{
	int input;
	RBTree *rb = new RBTree();

	while (1)
	{
		std::cout << "입력해라 애송이>>";
		std::cin >> input;
		rb->InsertNode(input);

		std::cout << std::endl;
	}
}