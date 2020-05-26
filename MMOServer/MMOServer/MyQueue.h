#pragma once

#include "MemoryPool.h"

template <class T>
class MyQueue
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
public:
	MyQueue();
	MyQueue(int maxCount);
	bool Enqueue(T data);
	bool Dequeue(T *data = NULL);
	int GetUseCount() { return _useCount; }
private:
	NODE * volatile _head;
	NODE * volatile _tail;

	int _useCount;
	int _maxCount;

	MemoryPool<NODE> *queuePool;
};

template<class T>
MyQueue<T>::MyQueue()
	:_maxCount(0),_useCount(0)
{
	queuePool = new MemoryPool<NODE>();
	_head = queuePool->Alloc();
	_tail = _head;
}

template<class T>
MyQueue<T>::MyQueue(int maxCount)
	:_useCount(0), _maxCount(maxCount)
{
	queuePool = new MemoryPool<NODE>(maxCount, true);
	_head = queuePool->Alloc();
	_tail->node = _head;
}

template<class T>
bool  MyQueue<T>::Enqueue(T data)
{
	NODE *newNode = queuePool->Alloc();
	newNode->item = data;

	if (newNode == NULL)
		return false;

	_tail->next = newNode;
	_tail = newNode;
	InterlockedIncrement((LONG *)&_useCount);

	return true;
}

template<class T>
bool  MyQueue<T>::Dequeue(T *data)
{
	NODE *temp;
	//isEmpty를 atomic할 수 있게
	if (InterlockedDecrement((LONG *)&_useCount) < 0)
	{
		InterlockedIncrement((LONG *)&_useCount);
		return false;
	}

	temp = _head;
	_head = _head->next;

	if (data != NULL)
		*data = _head->item;

	InterlockedDecrement((LONG *)&_useCount);

	return true;
}