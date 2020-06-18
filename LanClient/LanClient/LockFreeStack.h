#pragma once
#include <Windows.h>
#include "MemoryPool.h"

//최근 50000개만 기록한다
//맨앞 숫자(16진수) 1일때 free
//맨앞 숫자(16진수) 0일때 new
//#define TRACK_MAX 100000
//
//void *track[TRACK_MAX];
//unsigned long long trackCur = 0;

template<class T>
class LockFreeStack
{
private:
	struct NODE
	{
		NODE(T data)
		{
			next = NULL;
			item = data;
		}

		NODE()
		{
			next = NULL;
		}

		T item;
		NODE *next;
	};

	struct TOP
	{
		TOP()
		{
			node = NULL;
			//node=new NODE();
			//node->next = node;
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
	TOP * volatile _TopNode;
	unsigned long long _checkNum;
	MemoryPool<NODE> stackPool;

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
		//_useCount가 0으로 확인 후 Push가 들어왔을 경우가 있을 수 있으므로 체크
		if(_TopNode->node==NULL)
			return true;
	}
	return false;
}

template<class T>
bool LockFreeStack<T>::Push(T data)
{
	//int temp = InterlockedIncrement64(&_checkNum);
	//NODE *newNode = new NODE(data);
	NODE *newNode = stackPool.Alloc();

	if (newNode == NULL)
		return false;

	newNode->item = data;

	//추적용
	//ULONG trackTemp = InterlockedIncrement((LONG *)&trackCur);
	//InterlockedExchange64((LONG64 *)&track[trackTemp % TRACK_MAX], (LONG64)newNode);
	//InterlockedIncrement((LONG *)&trackCur);
	//추적용

	TOP t;
	
	unsigned long long checkNum = InterlockedIncrement64((LONG64 *)&_checkNum);//이 push행위의 checkNum은 함수 시작 시에 결정
	//do
	//{
	//	t.node = _TopNode->node;
	//	t.check = _TopNode->check;
	//	newNode->next = t.node;
	//	
	//} while (!InterlockedCompareExchange128((LONG64 *)_TopNode, (LONG64)checkNum, (LONG64)newNode, (LONG64 *)&t));

	
	//interlockedCompareExchange128이 실패할 시 비교 
	//피연산자인 t값에 대상의 값인 _TopNode가 덮어씌워지므로 do while방식과 같게 돌아가게 된다
	while (!InterlockedCompareExchange128((LONG64 *)_TopNode, (LONG64)checkNum, (LONG64)newNode, (LONG64 *)&t))
	{
		newNode->next = t.node;
		
	}
	InterlockedIncrement((LONG *)&_useCount);
	
	return true;
}


template<class T>
bool LockFreeStack<T>::Pop(T *data)
{
	//isEmpty를 atomic할 수 있게
	if (InterlockedDecrement((LONG *)&_useCount) < 0)
	{
		InterlockedIncrement((LONG *)&_useCount);
		return false;
	}

	TOP t;
	NODE *newTop = NULL;// = _TopNode->node->next;
	T popData = NULL;;// = _TopNode->node->item;
	unsigned long long checkNum = InterlockedIncrement64((LONG64 *)&_checkNum);//이 pop행위의 checkNum은 함수 시작 시에 결정
	bool success = true;
	//do
	//{
	//	if (IsEmpty())
	//	{
	//		data = NULL;
	//		return false;
	//	}
	//
	//	t.node = _TopNode->node;
	//	t.check = _TopNode->check;
	//	newTop = _TopNode->node->next;
	//	popData = _TopNode->node->item;
	//} while (!InterlockedCompareExchange128((LONG64 *)_TopNode, (LONG64)checkNum, (LONG64)newTop, (LONG64 *)&t));

	


	//interlockedCompareExchange128이 실패할 시 비교 
	//피연산자인 t값에 대상의 값인 _TopNode가 덮어씌워지므로 do while방식과 같게 돌아가게 된다
	while (!InterlockedCompareExchange128((LONG64 *)_TopNode, (LONG64)checkNum, (LONG64)newTop, (LONG64 *)&t))
	{
		//1개의 데이터가 있고 2개의 pop이 들어왔을 때
		//루프마다 isEmpty를 확인하지 않을 경우 1개의 pop이 완료된 후 2번째 pop은 데이터가 없을 때 pop을 시도하게됨
		//if (IsEmpty())
		//{
		//	data = NULL;
		//	return false;
		//}

		newTop = (NODE *)_TopNode->node->next;

		popData = _TopNode->node->item;
		
	} 

	//추적용
	//ULONG trackTemp = InterlockedIncrement((LONG *)&trackCur);
	//InterlockedExchange64((LONG64 *)&track[trackTemp % TRACK_MAX], (LONG64)t.node | 0x1000000000000000);
	//추적용
	
	//delete t.node;
	stackPool.Free(t.node);

	//top -> next가 자기 자신인 경우는 최초 생성시 만들어진 top뿐이다.

	if(data!=NULL)
		*data = popData;

	return true;
}