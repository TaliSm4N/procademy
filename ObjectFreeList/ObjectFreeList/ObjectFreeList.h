#pragma once
/*---------------------------------------------------------------

Ŭ���� �̸��� �ɹ� �Լ��� ���� �̹Ƿ� ������ �̸����� �����Ͽ� ��� ���ּ���

procademy MemoryPool.

�޸� Ǯ Ŭ���� (������Ʈ Ǯ / ��������Ʈ)
Ư�� ����Ÿ(����ü,Ŭ����,����)�� ������ �Ҵ� �� ��������.

- ����.

procademy::CMemoryPool<DATA> MemPool(300, FALSE);
DATA *pData = MemPool.Alloc();

pData ���

MemPool.Free(pData);


----------------------------------------------------------------*/
#ifndef  __OBJECT_FREE_LIST__
#define  __OBJECT_FREE_LIST__
#include <new.h>
#define TOP_BUMP 0xffffffff
#define BOTTOM_BUMP 0xfefefefe


template <class DATA>
class ObjectFreeList
{
private:

	/* **************************************************************** */
	// �� �� �տ� ���� ��� ����ü.
	/* **************************************************************** */
	struct NODE
	{
		NODE()
		{
			NextBlock = NULL;
			_topBump = TOP_BUMP;
			_bottomBump = BOTTOM_BUMP;
		}

		NODE *NextBlock;
		int _topBump;
		DATA item;
		int _bottomBump;
	};




public:

	//////////////////////////////////////////////////////////////////////////
	// ������, �ı���.
	//
	// Parameters: (int) �ʱ� �� ����.
	// (bool) Alloc �� ������ / Free �� �ı��� ȣ�� ����
	// Return:
	//////////////////////////////////////////////////////////////////////////
	ObjectFreeList(int iBlockNum, bool bPlacementNew = false);
	virtual ~ObjectFreeList();


	//////////////////////////////////////////////////////////////////////////
	// �� �ϳ��� �Ҵ�޴´�.  
	//
	// Parameters: ����.
	// Return: (DATA *) ����Ÿ �� ������.
	//////////////////////////////////////////////////////////////////////////
	DATA *Alloc(void);

	//////////////////////////////////////////////////////////////////////////
	// ������̴� ���� �����Ѵ�.
	//
	// Parameters: (DATA *) �� ������.
	// Return: (BOOL) TRUE, FALSE.
	//////////////////////////////////////////////////////////////////////////
	bool Free(DATA *pData);


	//////////////////////////////////////////////////////////////////////////
	// ���� Ȯ�� �� �� ������ ��´�. (�޸�Ǯ ������ ��ü ����)
	//  capacity size  
	//
	// Parameters: ����.
	// Return: (int) �޸� Ǯ ���� ��ü ����
	//////////////////////////////////////////////////////////////////////////
	int capacity(void) const { return _capacity; }

	//////////////////////////////////////////////////////////////////////////
	// ���� ������� �� ������ ��´�.
	//
	// Parameters: ����.
	// Return: (int) ������� �� ����.
	//////////////////////////////////////////////////////////////////////////
	int GetUseCount(void) const { return _useCount; }

private:
	// ���� ������� ��ȯ�� (�̻��) ������Ʈ ���� ����.

	NODE *_TopNode;
	int _capacity;
	int _useCount;
	bool _placeMentNew;
};

template <class DATA>
ObjectFreeList<DATA>::ObjectFreeList(int iBlockNum, bool bPlacementNew)
	:_TopNode(nullptr), _placeMentNew(bPlacementNew)
{
	_capacity = iBlockNum;
	_useCount = 0;
	if (iBlockNum > 0)
	{
		NODE *temp;
		for (int i = 0; i < iBlockNum; i++)
		{
			//temp = new NODE();
			temp = (NODE *)malloc(sizeof(NODE));

			if (_TopNode == nullptr)
			{
				temp->NextBlock = nullptr;
				_TopNode = temp;
			}
			else
			{
				temp->NextBlock = _TopNode;
				_TopNode = temp;
			}

			if (_placeMentNew)
			{
				new (&(temp->item)) DATA();
			}
		}
	}
}

template <class DATA>
ObjectFreeList<DATA>::~ObjectFreeList()
{
	NODE *temp;
	while (_TopNode != nullptr)
	{
		if (_placeMentNew)
		{
			_TopNode->item.~DATA();
			//delete (&(_TopNode->item));
		}
		temp = _TopNode;
		_TopNode = temp->NextBlock;
		delete temp;
	}
}

template <class DATA>
DATA *ObjectFreeList<DATA>::Alloc(void)
{
	NODE *ret;
	if (_capacity > 0)
	{
		ret = _TopNode;
		_TopNode = ret->NextBlock;
		_useCount++;
		_capacity--;
	}
	else
	{
		ret = new NODE();
	}

	if (_placeMentNew)
	{
		new (&(ret->item)) DATA();
	}

	return &(ret->item);
}

template <class DATA>
bool ObjectFreeList<DATA>::Free(DATA *pData)
{
	NODE *temp=(NODE *)((int)pData-sizeof(int)-sizeof(NODE *));
	
	if (temp->_topBump != TOP_BUMP || temp->_bottomBump != BOTTOM_BUMP)
	{
		return false;
	}

	if (_placeMentNew)
	{
		_TopNode->item.~DATA();
	}

	temp->NextBlock = _TopNode;
	_TopNode = temp;

	_useCount--;
	_capacity++;

	return true;
}

#endif
