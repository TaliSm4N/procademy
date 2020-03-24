#pragma once
#include <Windows.h>

template<class T>
class LockFreeStack
{
private:
	struct CAS;
	struct NODE
	{
		NODE(T data)
		{
			next = NULL;
			item = data;
		}
		T item;
		NODE *next;
	};

	struct TOP
	{
		TOP()
		{
			node = NULL;
			check = 0;
		}

		NODE *node;
		long long check;
	};

public:
	LockFreeStack();
	~LockFreeStack() {};
	T Top();
	bool Push(T data);
	bool Pop(T *data = NULL);
	bool IsEmpty();
	int GetUseCount() { return _useCount; }

private:
	TOP *_TopNode;
	long long _checkNum;

	int _useCount;
};

template<class T>
LockFreeStack<T>::LockFreeStack()
	:_checkNum(0), _useCount(0)
{
	_TopNode = new TOP();
}

template<class T>
T LockFreeStack<T>::Top()
{
	return _TopNode->node->item;
}

template<class T>
bool LockFreeStack<T>::IsEmpty()
{
	if (_useCount == 0)
	{
		//_useCount가 0인 상태에서 Push가 들어왔을 경우가 있을 수 있으므로 체크
		if(_TopNode->node==NULL)
			return true;
	}
	return false;
}

template<class T>
bool LockFreeStack<T>::Push(T data)
{
	//int temp = InterlockedIncrement64(&_checkNum);
	NODE *newNode = new NODE(data);

	if (newNode == NULL)
		return false;

	TOP t;
	
	int checkNum = InterlockedIncrement((LONG *)&_checkNum);
	do
	{
		InterlockedExchange64((LONG64 *)&t, *((LONG64 *)_TopNode));
		newNode->next = t.node;
		
	} while (!InterlockedCompareExchange128((LONG64 *)_TopNode, (LONG64)checkNum, (LONG64)newNode, (LONG64 *)&t));

	InterlockedIncrement((LONG *)&_useCount);
	
	return true;
}


template<class T>
bool LockFreeStack<T>::Pop(T *data)
{
	TOP t;
	NODE *newTop;
	int checkNum = InterlockedIncrement((LONG *)&_checkNum);
	T popData;
	do
	{
		if (IsEmpty())
			return false;

		InterlockedExchange64((LONG64 *)&t, *((LONG64 *)_TopNode));
		newTop = _TopNode->node->next;
		popData = _TopNode->node->item;
	} while (!InterlockedCompareExchange128((LONG64 *)_TopNode, (LONG64)checkNum, (LONG64)newTop, (LONG64 *)&t));
	InterlockedDecrement((LONG *)&_useCount);
	delete t.node;
	if(data!=NULL)
		*data = popData;

	return true;
}