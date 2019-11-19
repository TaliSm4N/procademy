#pragma once
#include <iostream>
template <typename T>
class CList
{
public:
	struct Node
	{
		T _data;
		Node *_Prev;
		Node *_Next;
	};

	class iterator
	{
	private:
		Node *_node;
	public:
		iterator(Node *node = nullptr):_node(node)
		{
		}

		iterator& operator ++()
		{
			this->_node = this->_node->_Next;
			return *this;
		}

		iterator& operator ++(int)
		{
			iterator temp(this->_node);
			this->_node = this->_node->_Next;

			//return this;
			return temp;
		}

		bool operator ==(iterator iter)
		{
			if (this->_node == iter._node)
				return true;
			else
				return false;
		}

		bool operator !=(iterator iter)
		{
			if (this->_node != iter._node)
				return true;
			else
				return false;
		}

		T& operator *()
		{
			return this->_node->_data;
			//���� ����� �����͸� ����
		}
	};

public:
	CList() 
	{
		_head._Next = &_tail;
		_head._Prev = nullptr;
		_tail._Next = nullptr;
		_tail._Prev = &_head;
	};
	~CList()
	{
		clear();
	};

	iterator begin()
	{
		iterator iter(_head._Next);

		return iter;
		//ù��° ��带 ����Ű�� ���ͷ����� ����
	}
	iterator end()
	{
		iterator iter(&_tail);
		return iter;
		//Tail ��带 ����Ű��(�����Ͱ� ���� ��¥ �� ���) ���ͷ����͸� ����
		//	�Ǵ� ������ ������ �� �ִ� ���ͷ����͸� ����
	}

	void push_front(T data);
	void push_back(T data);
	void clear();
	int size() { return _size; };
	bool empty() { if (_size == 0) return true; else return false; };

	iterator erase(iterator iter)
	{
		iterator tempIter=iter++;
		//iter._node->_Prev->_Next = iter._node->_Next;
		//iter._node->_Next->_Prev = iter._node->_Prev;
		//free(iter._node);
		return iter;
		//-���ͷ������� �� ��带 ����.
		//- �׸��� ���� ����� ���� ��带 ī��Ű�� ���ͷ����� ����
	}


private:
	unsigned int _size = 0;
	Node _head;
	Node _tail;
};

template <typename T>
void CList<T>::push_front(T data)
{
	Node *newNode = (Node *)malloc(sizeof(Node));
	newNode->_data = data;

	
	newNode->_Next = _head._Next;
	newNode->_Prev = &_head;
	_head._Next->_Prev = newNode;
	_head._Next = newNode;
	
	_size++;
}

template <typename T>
void CList<T>::push_back(T data)
{
	Node *newNode = (Node *)malloc(sizeof(Node));
	newNode->_data = data;

	newNode->_Prev = _tail._Prev;
	newNode->_Next = &_tail;
	_tail._Prev->_Next = newNode;
	_tail._Prev = newNode;

	_size++;
}

template <typename T>
void CList<T>::clear()
{
	if (!empty())
	{
		iterator iter=this->begin();

		while (iter != this->end())
		{
			iter = erase(iter);
		}

	}
}