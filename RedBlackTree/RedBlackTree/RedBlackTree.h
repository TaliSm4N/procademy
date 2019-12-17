#pragma once

#define RBTree RedBlackTree

class RedBlackTree
{
public:
	enum NODE_COLOR
	{
		BLACK = 0,
		RED
	};

	struct NODE
	{
		struct NODE *pParent;
		struct NODE *pLeft;
		struct NODE *pRight;

		NODE_COLOR Color;

		int key;
	};

public:
	RedBlackTree();
	~RedBlackTree();

	bool InsertNode(int iData);
	bool DeleteNode(int iData);
	NODE *SearchNode(int iData);
	bool ReleaseTree(void);

	void Print();
	
private:
	bool InsertBalance(NODE *newNode);
	bool RightRotate(NODE *pivotNode);
	bool LeftRotate(NODE *pivotNode);
	bool DeleteBalance(NODE *pNode);

private:
	NODE *rootNode;
	NODE *NIL;
}; 



