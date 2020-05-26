#pragma once

#include "CrashDump.h"
#include "Profiler.h"

#include <Windows.h>
#include "MemoryPool.h"

//최근 50000개만 기록한다
//맨앞 숫자(16진수) 1일때 dequeue
//맨앞 숫자(16진수) 0일때 enqueue
#define TRACK_MAX 100000

void *track[TRACK_MAX];
unsigned long long trackCur = 0;

template <class T>
class LockFreeQueue
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

	struct END_NODE
	{
		END_NODE()
		{
			node = NULL;
			check = 0;
		}

		NODE *node;
		long long check;
	};
public:
	LockFreeQueue();
	bool Enqueue(T data);
	bool Dequeue(T *data = NULL);
	unsigned long long GetUseCount() { return _useCount; }
private:
	END_NODE * volatile _head;
	END_NODE * volatile _tail;

	unsigned long long _headCheckNum;
	unsigned long long _tailCheckNum;

	int _useCount;
	int _maxCount;

	MemoryPool<NODE> queuePool;
};

template<class T>
LockFreeQueue<T>::LockFreeQueue()
	:_headCheckNum(0), _tailCheckNum(0), _useCount(0),_maxCount(0)
{
	_head = new END_NODE;
	_head->node = queuePool.Alloc();
	_tail = new END_NODE;
	_tail->node = _head->node;
}

template<class T>
bool LockFreeQueue<T>::Enqueue(T data)
{
	PRO_BEGIN(L"ENQ");
	NODE *newNode = queuePool.Alloc();
	

	//추적용
	//ULONG trackTemp = InterlockedIncrement((LONG *)&trackCur);
	//InterlockedExchange64((LONG64 *)&track[trackTemp % TRACK_MAX], (LONG64)newNode);
	//추적용

	if (newNode == NULL)
	{
		PRO_END(L"ENQ");
		return false;
	}

	newNode->item = data;

	
	unsigned long long checkNum = InterlockedIncrement64((LONG64 *)&_tailCheckNum);
		

	while (1)
	{
		END_NODE tail;// = _tail;
		tail.check = _tail->check;
		tail.node = _tail->node;

		if (tail.node->next == (NODE *)0x1)
			continue;

		NODE *next = tail.node->next;

		if (next == NULL)
		{
			//tail의 next가 그대로 NULL일 때 진입
			if (InterlockedCompareExchangePointer((PVOID *)&tail.node->next, newNode, next) == NULL)
			{
				//_tail은 새로운 node가 됨
				//실패한다면 이 시점에 서는 _tail이 이미 변경됬음 => 변경된 곳에서 _tail을 맞춰줄 것을 기대한다
				//위의 interlock과는 독립적으로 돌아가기때문에 _tail뒤에는 이미 새로운 node가 들어왔을 가능성이 있음
				//(하지만 _tail은 여전히 현재 tail을 가리키므로 _tail의 next가 null이 아니게됨)
				InterlockedCompareExchange128((LONG64 *)_tail, checkNum, (LONG64)tail.node->next, (LONG64 *)&tail);

				break;
			}
		}
		else
		{
			//tail의 next가 NULL이 아닌 경우 tail을 옮겨줘야함
			//단 이경우에도 _tail은 atomic하게 변경되어야함
		
			InterlockedCompareExchange128((LONG64 *)_tail, checkNum, (LONG64)tail.node->next, (LONG64 *)&tail);
			checkNum = InterlockedIncrement64((LONG64 *)&_tailCheckNum);//_tail이 변경됨에 따라서 checkNum도 변경
		}
	}

	InterlockedIncrement((LONG *)&_useCount);
	PRO_END(L"ENQ");
	return true;
}

template<class T>
bool LockFreeQueue<T>::Dequeue(T *data)
{
	//isEmpty를 atomic할 수 있게
	if (InterlockedDecrement((LONG *)&_useCount) < 0)
	{
		InterlockedIncrement((LONG *)&_useCount);
		return false;
	}

	PRO_BEGIN(L"DEQ");
	END_NODE h;
	//NODE *newHead = NULL;
	T popData=NULL;
	unsigned long long checkNum = InterlockedIncrement64((LONG64 *)&_headCheckNum);//이 pop행위의 checkNum은 함수 시작 시에 결정


	while (1)
	{

		h.check = _head->check;
		h.node = _head->node;
		NODE *next = h.node->next;

		//tail이 밀리지 않았을 때 모든 head를 빼내게 될 경우 tail이 유실될 수 있다.
		END_NODE tail;// = _tail;
		tail.check = _tail->check;
		tail.node = _tail->node;
		NODE *tailNext = tail.node->next;

		if (tail.node->next != NULL)
		{

			unsigned long long tailCheckNum = InterlockedIncrement64((LONG64 *)&_tailCheckNum);//_tail이 변경됨에 따라서 checkNum도 변경
		
			InterlockedCompareExchange128((LONG64 *)_tail, tailCheckNum, (LONG64)tail.node->next, (LONG64 *)&tail);

			continue;
		}

		if (next == NULL)
			continue;

		popData = next->item;

		if (InterlockedCompareExchange128((LONG64 *)_head, checkNum, (LONG64)h.node->next, (LONG64 *)&h))
		{
			if (data != NULL)
				*data = popData;
			break;
		}
	}

	
	//추적용
	//ULONG trackTemp = InterlockedIncrement((LONG *)&trackCur);
	//InterlockedExchange64((LONG64 *)&track[trackTemp % TRACK_MAX], (LONG64)h.node | 0x1000000000000000);
	//추적용
	
	queuePool.Free(h.node);

	PRO_END(L"DEQ");
	return true;
}