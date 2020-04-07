#pragma once

#include <Windows.h>
#include <new.h>
#define TOP_BUMP 0xffffffff
#define BOTTOM_BUMP 0xfefefefe


template <class T>
class MemoryPool
{
private:
//#pragma pack(push,1)
	struct NODE
	{
		NODE()
		{
			NextBlock = NULL;
			_bottomBump = BOTTOM_BUMP;
		}

		NODE *NextBlock;
		T item;
		int _bottomBump;
	};
//#pragma pack(pop)

	struct TOP
	{
		TOP()
		{
			node = NULL;
			check = 0;
		}

		NODE *node;
		unsigned long long check;
	};

public:
	MemoryPool(int blockNum=0,bool maxLimit=false);
	~MemoryPool();

	T *Alloc(bool placeMent = true);

	bool Free(T *data);

	int GetUseCount() { return _useCount; }

private:
	TOP *_topNode;
	int _useCount;
	int _maxCapacity;
	bool _maxLimit;
	int _freeCount;
	unsigned long long _checkNum;

public:
	int test;
};


template <class T>
MemoryPool<T>::MemoryPool(int blockNum,bool maxLimit)
	:_maxCapacity(blockNum),_checkNum(0),test(0),_freeCount(blockNum)
{
	NODE *temp;
	if (blockNum == 0)
	{
		_maxLimit = false;
	}
	else
	{
		_maxLimit = maxLimit;
	}

	_topNode = new TOP();

	for (int i = 0; i < blockNum; i++)
	{
		temp = (NODE *)malloc(sizeof(NODE));
		temp->_bottomBump = BOTTOM_BUMP;
		//temp = new NODE();

		if (_topNode->node == nullptr)
		{
			temp->NextBlock = nullptr;
			_topNode->node = temp;
		}
		else
		{
			temp->NextBlock = _topNode->node;
			_topNode->node = temp;
		}
		//if(_placeMent)
		//	new (&(temp->item)) T();
	}
	_topNode->check = 0;
}

template <class T>
MemoryPool<T>::~MemoryPool()
{
	NODE *cur = _topNode->node;
	NODE *temp;
	while (cur != NULL)
	{
		temp = cur;
		cur = cur->NextBlock;

		delete temp;
	}
	delete _topNode;
}

template<class T>
T *MemoryPool<T>::Alloc(bool placement)
{
	NODE *ret = NULL;
	NODE *newTop=NULL;
	TOP t;
	unsigned long long checkNum;



	InterlockedIncrement((LONG *)&test);

	//if (_maxCapacity < InterlockedIncrement((LONG *)&_useCount))
	if(InterlockedDecrement((LONG *)&_freeCount)<0)
	{
		//maxLimit가 true인 경우 새로운 node를 생성하지 않음
		if (_maxLimit)
		{
			//InterlockedDecrement((LONG *)&_useCount);
			return NULL;
		}
		else // maxLimit가 true가 아닌 경우 새로운 node를 생성해서 전달
		{

			InterlockedIncrement((LONG *) &_maxCapacity);
			InterlockedIncrement((LONG *)&_freeCount);
			//ret = new NODE();
			ret = (NODE *)malloc(sizeof(NODE));
			ret->_bottomBump = BOTTOM_BUMP;
			
			if (placement)
				new (&(ret->item)) T();

			return &(ret->item);
		}

		volatile int test = 1;
	}

	checkNum = InterlockedIncrement64((LONG64 *)&_checkNum);//이 pop행위의 checkNum은 함수 시작 시에 결정

	while (!InterlockedCompareExchange128((LONG64 *)_topNode, (LONG64)checkNum, (LONG64)newTop, (LONG64 *)&t))
	{
		newTop = (NODE *)_topNode->node->NextBlock;

		ret = _topNode->node;
	}

	if (ret == NULL)
		return NULL;

	if (placement)
		new (&(ret->item)) T();

	return &(ret->item);
}

template<class T>
bool MemoryPool<T>::Free(T *data)
{
	//InterlockedIncrement((LONG *)&test);

	NODE *temp = (NODE *)((LONG64)data-sizeof(NODE *));
	TOP t;
	unsigned long long checkNum;

	if (temp->_bottomBump != BOTTOM_BUMP)
	{
		return false;
	}

	checkNum = InterlockedIncrement64((LONG64 *)&_checkNum);//이 push행위의 checkNum은 함수 시작 시에 결정
	
	while (!InterlockedCompareExchange128((LONG64 *)_topNode, (LONG64)checkNum, (LONG64)temp, (LONG64 *)&t))
	{
		temp->NextBlock = t.node;
	}
	//InterlockedDecrement((LONG *)&_useCount);
	InterlockedIncrement((LONG *)&_freeCount);

	return true;
}