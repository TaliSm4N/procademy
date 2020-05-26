#pragma once

#include "CrashDump.h"
#include "Profiler.h"

#include <Windows.h>
#include "MemoryPool.h"

//�ֱ� 50000���� ����Ѵ�
//�Ǿ� ����(16����) 1�϶� dequeue
//�Ǿ� ����(16����) 0�϶� enqueue
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
	

	//������
	//ULONG trackTemp = InterlockedIncrement((LONG *)&trackCur);
	//InterlockedExchange64((LONG64 *)&track[trackTemp % TRACK_MAX], (LONG64)newNode);
	//������

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
			//tail�� next�� �״�� NULL�� �� ����
			if (InterlockedCompareExchangePointer((PVOID *)&tail.node->next, newNode, next) == NULL)
			{
				//_tail�� ���ο� node�� ��
				//�����Ѵٸ� �� ������ ���� _tail�� �̹� ������� => ����� ������ _tail�� ������ ���� ����Ѵ�
				//���� interlock���� ���������� ���ư��⶧���� _tail�ڿ��� �̹� ���ο� node�� ������ ���ɼ��� ����
				//(������ _tail�� ������ ���� tail�� ����Ű�Ƿ� _tail�� next�� null�� �ƴϰԵ�)
				InterlockedCompareExchange128((LONG64 *)_tail, checkNum, (LONG64)tail.node->next, (LONG64 *)&tail);

				break;
			}
		}
		else
		{
			//tail�� next�� NULL�� �ƴ� ��� tail�� �Ű������
			//�� �̰�쿡�� _tail�� atomic�ϰ� ����Ǿ����
		
			InterlockedCompareExchange128((LONG64 *)_tail, checkNum, (LONG64)tail.node->next, (LONG64 *)&tail);
			checkNum = InterlockedIncrement64((LONG64 *)&_tailCheckNum);//_tail�� ����ʿ� ���� checkNum�� ����
		}
	}

	InterlockedIncrement((LONG *)&_useCount);
	PRO_END(L"ENQ");
	return true;
}

template<class T>
bool LockFreeQueue<T>::Dequeue(T *data)
{
	//isEmpty�� atomic�� �� �ְ�
	if (InterlockedDecrement((LONG *)&_useCount) < 0)
	{
		InterlockedIncrement((LONG *)&_useCount);
		return false;
	}

	PRO_BEGIN(L"DEQ");
	END_NODE h;
	//NODE *newHead = NULL;
	T popData=NULL;
	unsigned long long checkNum = InterlockedIncrement64((LONG64 *)&_headCheckNum);//�� pop������ checkNum�� �Լ� ���� �ÿ� ����


	while (1)
	{

		h.check = _head->check;
		h.node = _head->node;
		NODE *next = h.node->next;

		//tail�� �и��� �ʾ��� �� ��� head�� ������ �� ��� tail�� ���ǵ� �� �ִ�.
		END_NODE tail;// = _tail;
		tail.check = _tail->check;
		tail.node = _tail->node;
		NODE *tailNext = tail.node->next;

		if (tail.node->next != NULL)
		{

			unsigned long long tailCheckNum = InterlockedIncrement64((LONG64 *)&_tailCheckNum);//_tail�� ����ʿ� ���� checkNum�� ����
		
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

	
	//������
	//ULONG trackTemp = InterlockedIncrement((LONG *)&trackCur);
	//InterlockedExchange64((LONG64 *)&track[trackTemp % TRACK_MAX], (LONG64)h.node | 0x1000000000000000);
	//������
	
	queuePool.Free(h.node);

	PRO_END(L"DEQ");
	return true;
}