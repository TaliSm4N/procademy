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
	TOP *_TopNode;
	unsigned long long _checkNum;

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
		//_useCount�� 0���� Ȯ�� �� Push�� ������ ��찡 ���� �� �����Ƿ� üũ
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
	
	unsigned long long checkNum = InterlockedIncrement64((LONG64 *)&_checkNum);//�� push������ checkNum�� �Լ� ���� �ÿ� ����
	//do
	//{
	//	t.node = _TopNode->node;
	//	t.check = _TopNode->check;
	//	newNode->next = t.node;
	//	
	//} while (!InterlockedCompareExchange128((LONG64 *)_TopNode, (LONG64)checkNum, (LONG64)newNode, (LONG64 *)&t));

	
	//interlockedCompareExchange128�� ������ �� �� 
	//�ǿ������� t���� ����� ���� _TopNode�� ��������Ƿ� do while��İ� ���� ���ư��� �ȴ�
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
	//if (IsEmpty())
	//	return false;

	TOP t;
	NODE *newTop = NULL;// = _TopNode->node->next;
	T popData = NULL;;// = _TopNode->node->item;
	unsigned long long checkNum = InterlockedIncrement64((LONG64 *)&_checkNum);//�� pop������ checkNum�� �Լ� ���� �ÿ� ����
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


	//interlockedCompareExchange128�� ������ �� �� 
	//�ǿ������� t���� ����� ���� _TopNode�� ��������Ƿ� do while��İ� ���� ���ư��� �ȴ�
	while (!InterlockedCompareExchange128((LONG64 *)_TopNode, (LONG64)checkNum, (LONG64)newTop, (LONG64 *)&t))
	{
		//1���� �����Ͱ� �ְ� 2���� pop�� ������ ��
		//�������� isEmpty�� Ȯ������ ���� ��� 1���� pop�� �Ϸ�� �� 2��° pop�� �����Ͱ� ���� �� pop�� �õ��ϰԵ�
		if (IsEmpty())
		{
			data = NULL;
			return false;
		}
		//isEmpty�� Ȯ���� ���� stack�� �������� �Ǹ� �ؿ� �ڵ� �ۻ쳲
		//�� ���¿����� push�� �ͺ��� pop�� �� �����ϸ� ���а� �ƴ϶� crash�� �� Ȯ���� ������

		newTop = _TopNode->node->next;

		popData = _TopNode->node->item;
		
	} 


	InterlockedDecrement((LONG *)&_useCount);
	delete t.node;

	//top -> next�� �ڱ� �ڽ��� ���� ���� ������ ������� top���̴�.

	if(data!=NULL)
		*data = popData;

	return true;
}