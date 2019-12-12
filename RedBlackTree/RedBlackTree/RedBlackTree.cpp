#include "RedBlackTree.h"

RedBlackTree::RedBlackTree()
{
	NIL = new NODE();
	NIL->Color = BLACK;
	NIL->pParent = nullptr;
	NIL->pLeft = nullptr;
	NIL->pRight = nullptr;
	rootNode = nullptr;
}

RedBlackTree::~RedBlackTree()
{
}

bool RedBlackTree::InsertNode(int iData)
{
	NODE *newNode = new NODE();
	NODE *curNode=rootNode;
	bool leftSide = false;
	newNode->Color = RED;
	newNode->key = iData;
	newNode->pLeft = NIL;
	newNode->pRight = NIL;
	
	if (rootNode == nullptr)
	{
		newNode->pParent = nullptr;
		rootNode = newNode;
	}
	else
	{
		//자리 찾기
		while (1)
		{
			if (curNode->key < iData)
			{
				if (curNode->pRight != NIL)
				{
					curNode = curNode->pRight;
				}
				else
				{
					leftSide = false;
					break;
				}
			}
			else if (curNode->key > iData)
			{
				if (curNode->pLeft != NIL)
				{
					curNode = curNode->pLeft;
					leftSide = false;
				}
				else
				{
					leftSide = true;
					break;
				}
			}
			else
			{
				return false;
			}
		}

		//노드 삽입
		if (leftSide)
		{
			curNode->pLeft = newNode;
			newNode->pParent = curNode;
		}
		else
		{
			curNode->pRight = newNode;
			newNode->pParent = curNode;
		}
		Balance(newNode);
	}
	rootNode->Color = BLACK;
	return true;
}

RedBlackTree::NODE *RedBlackTree::SearchNode(int iData)
{
	return NIL;
}

bool RedBlackTree::Balance(NODE *newNode)
{
	NODE *parent= newNode->pParent;
	NODE *gParent;
	bool pLeft = false;
	bool gpLeft = false;
	if (parent->Color == BLACK)
	{
		return true;
	}

	gParent = parent->pParent;

	//삼촌 부모 모두 red
	//부모가 BLACK일 경우 여기까지 함수가 도달하지 않으므로 둘이 같다면 둘다 RED
	if (gParent->pLeft->Color == gParent->pRight->Color)
	{
		gParent->Color = RED;
		gParent->pLeft->Color = BLACK;
		gParent->pRight->Color = BLACK;

		if(gParent!=rootNode)
			Balance(gParent);
	}
	else
	{
		if (parent->pLeft == newNode)
			pLeft = true;
		if (gParent->pLeft == parent)
			gpLeft = true;

		//한쪽으로 쏠리지 않은 경우
		if (pLeft != gpLeft)
		{
			if (pLeft)
			{
				RightRotate(parent);
			}
			else
			{
				LeftRotate(parent);
			}

			newNode = parent;
			parent = newNode->pParent;
		}

		gParent->Color = RED;
		parent->Color = BLACK;

		if (gpLeft)
			RightRotate(gParent);
		else
			LeftRotate(gParent);
	}
	/*
	else
	{
		if (parent->pLeft == newNode)
			pLeft = true;
		if (gParent->pLeft == parent)
			gpLeft = true;

		if (pLeft != gpLeft)
		{
			newNode->pParent = gParent;
			parent->pParent = newNode;

			if (pLeft)
			{
				parent->pLeft = newNode->pRight;
				gParent->pRight = newNode;
				newNode->pRight = parent;
			}
			else
			{
				parent->pRight = newNode->pLeft;
				gParent->pLeft = newNode;
				newNode->pLeft = parent;
			}

			newNode = parent;
			parent = newNode->pParent;
		}

		gParent->Color = RED;
		parent->Color = BLACK;

		if (gpLeft)
		{
			gParent->pLeft = parent->pRight;
			parent->pParent = gParent->pParent;
			parent->pRight = gParent;
			
			if (gParent == rootNode)
			{
				rootNode = parent;
			}
			else
			{
				if(pLeft)
					parent->pParent->pLeft = parent;
				else
					parent->pParent->pRight = parent;
			}
			gParent->pParent = parent;

		}
		else
		{
			gParent->pRight = parent->pLeft;
			parent->pParent = gParent->pParent;
			parent->pLeft = gParent;
			if (gParent == rootNode)
			{
				rootNode = parent;
			}
			else
			{
				if (pLeft)
					parent->pParent->pLeft = parent;
				else
					parent->pParent->pRight = parent;
			}
			gParent->pParent = parent;
		}
	}
	*/

	return true;
}

bool RedBlackTree::RightRotate(NODE *pivotNode)
{
	NODE * topNode = pivotNode->pLeft;
	pivotNode->pLeft = topNode->pRight;
	topNode->pRight = pivotNode;
	topNode->pParent = pivotNode->pParent;
	pivotNode->pParent = topNode;
	pivotNode->pLeft->pParent = pivotNode;

	if (pivotNode == rootNode)
	{
		rootNode = topNode;
	}
	else
	{
		if (topNode->pParent->pLeft == pivotNode)
		{
			topNode->pParent->pLeft = topNode;
		}
		else if (topNode->pParent->pRight == pivotNode)
		{
			topNode->pParent->pRight = topNode;
		}
	}

	return true;
}
bool RedBlackTree::LeftRotate(NODE *pivotNode)
{
	
	NODE * topNode = pivotNode->pRight;
	pivotNode->pRight = topNode->pLeft;
	topNode->pLeft = pivotNode;
	topNode->pParent = pivotNode->pParent;
	pivotNode->pParent = topNode;
	pivotNode->pRight->pParent = pivotNode;

	if (pivotNode == rootNode)
	{
		rootNode = topNode;
	}
	else
	{
		if (topNode->pParent->pLeft == pivotNode)
		{
			topNode->pParent->pLeft = topNode;
		}
		else if (topNode->pParent->pRight == pivotNode)
		{
			topNode->pParent->pRight = topNode;
		}
	}

	return true;
}