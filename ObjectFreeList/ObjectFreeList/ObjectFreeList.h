#pragma once
/*---------------------------------------------------------------

클래스 이름과 맴버 함수는 예시 이므로 적절한 이름으로 변경하여 사용 해주세요

procademy MemoryPool.

메모리 풀 클래스 (오브젝트 풀 / 프리리스트)
특정 데이타(구조체,클래스,변수)를 일정량 할당 후 나눠쓴다.

- 사용법.

procademy::CMemoryPool<DATA> MemPool(300, FALSE);
DATA *pData = MemPool.Alloc();

pData 사용

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
	// 각 블럭 앞에 사용될 노드 구조체.
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
	// 생성자, 파괴자.
	//
	// Parameters: (int) 초기 블럭 개수.
	// (bool) Alloc 시 생성자 / Free 시 파괴자 호출 여부
	// Return:
	//////////////////////////////////////////////////////////////////////////
	ObjectFreeList(int iBlockNum, bool bPlacementNew = false);
	virtual ~ObjectFreeList();


	//////////////////////////////////////////////////////////////////////////
	// 블럭 하나를 할당받는다.  
	//
	// Parameters: 없음.
	// Return: (DATA *) 데이타 블럭 포인터.
	//////////////////////////////////////////////////////////////////////////
	DATA *Alloc(void);

	//////////////////////////////////////////////////////////////////////////
	// 사용중이던 블럭을 해제한다.
	//
	// Parameters: (DATA *) 블럭 포인터.
	// Return: (BOOL) TRUE, FALSE.
	//////////////////////////////////////////////////////////////////////////
	bool Free(DATA *pData);


	//////////////////////////////////////////////////////////////////////////
	// 현재 확보 된 블럭 개수를 얻는다. (메모리풀 내부의 전체 개수)
	//  capacity size  
	//
	// Parameters: 없음.
	// Return: (int) 메모리 풀 내부 전체 개수
	//////////////////////////////////////////////////////////////////////////
	int capacity(void) const { return _capacity; }

	//////////////////////////////////////////////////////////////////////////
	// 현재 사용중인 블럭 개수를 얻는다.
	//
	// Parameters: 없음.
	// Return: (int) 사용중인 블럭 개수.
	//////////////////////////////////////////////////////////////////////////
	int GetUseCount(void) const { return _useCount; }

private:
	// 스택 방식으로 반환된 (미사용) 오브젝트 블럭을 관리.

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
