#include "RedBlackTree.h"

RedBlackTree::RedBlackTree()
{
	NIL = new NODE();
	NIL->Color = BLACK;
	NIL->pParent = NIL;
	NIL->pLeft = NIL;
	NIL->pRight = NIL;
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
		InsertBalance(newNode);
	}
	rootNode->Color = BLACK;
	return true;
}

bool RedBlackTree::DeleteNode(int iData)
{
	NODE *pNode = SearchNode(iData);
	NODE *curNode = pNode;
	bool left = false;
	bool right = false;

	if (pNode->pLeft != NIL)
		left=true;
	if (pNode->pRight != NIL)
		right = true;

	if (left)
	{
		curNode=pNode->pLeft;

		while (curNode->pRight != NIL)
		{
			curNode = curNode->pRight;
		}
	}
	else if (right)
	{
		curNode = pNode->pRight;

		while (curNode->pLeft != NIL)
		{
			curNode = curNode->pLeft;
		}
	}


	/*
	NODE *pNode = SearchNode(iData);
	NODE *curNode=pNode;
	bool leftChild = false;
	bool rightChild = false;

	if (pNode == NIL)
		return false;
	if (pNode->pLeft != NIL)
		leftChild = true;
	if (pNode->pRight != NIL)
		rightChild = true;

	if (leftChild || rightChild)
	{
		if (leftChild)
		{
			curNode = pNode->pLeft;

			while (curNode->pRight != NIL)
			{
				curNode = curNode->pRight;
			}
		}
		else if (rightChild)
		{
			curNode = pNode->pRight;

			while (curNode->pLeft != NIL)
			{
				curNode = curNode->pLeft;
			}
		}

		pNode->key = curNode->key;
		pNode = curNode;
	}

	

	if (pNode->pParent->pLeft == pNode)
	{
		pNode->pRight->pParent = pNode->pParent;
		pNode->pParent->pLeft = pNode->pRight;
		//delete pNode;
	}
	else
	{
		pNode->pLeft->pParent = pNode->pParent;
		pNode->pParent->pRight = pNode->pLeft;
		//delete pNode;
	}

	if (pNode->Color!=RED)
	{
		DeleteBalance(pNode);
	}
	delete pNode;

	*/
	/*
	NODE *pNode = SearchNode(iData);
	NODE *curNode;
	bool leftChild = false;
	bool rightChild = false;

	if (pNode == NIL)
		return false;
	if (pNode->pLeft != NIL)
		leftChild = true;
	if (pNode->pRight != NIL)
		rightChild = true;

	if(leftChild||rightChild)
	{
		if (leftChild)
		{
			curNode = pNode->pLeft;

			while (curNode->pRight!=NIL)
			{
				curNode = curNode->pRight;
			}
		}
		else if (rightChild)
		{
			curNode = pNode->pRight;

			while (curNode->pLeft != NIL)
			{
				curNode = curNode->pLeft;
			}
		}

		pNode->key = curNode->key;
		pNode = curNode;
	}

	Delete(pNode);
	*/
}

bool RedBlackTree::DeleteBalance(NODE *pNode)
{
	/*
	NODE *parent = pNode->pParent;
	NODE *brother;
	bool left;
	if (parent->pLeft == pNode)
	{
		brother = parent->pRight;
		left = true;
	}
	else
	{
		brother = parent->pLeft;
		left = false;
	}

	if (pNode->pLeft->Color == RED || pNode->pRight->Color == RED)
	{
		if (left)
		{
			pNode->pRight->Color = BLACK;
		}
		else
		{
			pNode->pLeft->Color = BLACK;
		}
	}
	else if (brother->Color == RED)
	{
		brother->Color = BLACK;

		if (left)
		{
			LeftRotate(parent);
			DeleteBalance(pNode->pRight);
		}
		else
		{
			RightRotate(parent);
			DeleteBalance(pNode->pLeft);
		}
	}
	else
	{
		if (brother->pLeft->Color == BLACK && brother->pRight->Color == BLACK)
		{
			brother->Color = BLACK;
			DeleteBalance(parent->pParent);
		}
		else
		{
			if (left)
			{
				if (brother->pLeft->Color == RED)
				{
					brother->pLeft->Color = BLACK;
					brother->Color = RED;
					RightRotate(brother);
					brother = brother->pParent;
				}

				brother->Color = parent->Color;
				parent->Color = BLACK;
				brother->pRight->Color = BLACK;

				LeftRotate(parent);
			}
			else
			{
				if (brother->pRight->Color == RED)
				{
					brother->pRight->Color = BLACK;
					brother->Color = RED;
					LeftRotate(brother);
					brother = brother->pParent;
				}

				brother->Color = parent->Color;
				parent->Color = BLACK;
				brother->pLeft->Color = BLACK;

				RightRotate(parent);
			}
		}
	}
	
	return true;
	*/
	/*
	NODE *parent = pNode->pParent;

	if (pNode->Color == RED)
	{
		if (parent->pLeft == pNode)
		{
			parent->pLeft = pNode->pLeft;
		}
		else if (parent->pRight == pNode)
		{
			parent->pRight = pNode->pRight;
		}

		delete pNode;
	}
	else
	{
		//삭제 노드의 형제가 레드
		if (parent->pLeft->Color != parent->pRight->Color)
		{
			if (parent->Color == RED)
			{
				if (parent->pLeft == pNode)
				{
					parent->pRight->Color = BLACK;
				}
				else
				{
					parent->pLeft->Color = BLACK;
				}
			}
			else
			{
				if (parent->pLeft == pNode)
				{
					parent->pRight->Color = BLACK;
					parent->Color = RED;
					LeftRotate(parent);
					Delete(pNode);
				}
				else
				{
					parent->pLeft->Color = BLACK;
					parent->Color = RED;
					RightRotate(parent);
					Delete(pNode);
				}
			}
		}
		else//삭제노드의 형제는 BLACK
		{
			NODE *brother;

			if()
		}
	}
	*/
}

RedBlackTree::NODE *RedBlackTree::SearchNode(int iData)
{
	NODE *curNode = rootNode;

	if (rootNode == nullptr)
		return NIL;

	while (1)
	{
		if (curNode->key < iData)
		{
			curNode = curNode->pRight;
		}
		else if (curNode->key > iData)
		{
			curNode = curNode->pLeft;
		}
		else if (curNode->key == iData)
		{
			break;
		}
		else if (curNode == NIL)
			break;
	}
	return curNode;
}

bool RedBlackTree::InsertBalance(NODE *newNode)
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
			InsertBalance(gParent);
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

void RedBlackTree::Print()
{
}