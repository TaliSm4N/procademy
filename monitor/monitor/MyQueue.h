#pragma once

#define dfSize	100

template <class T>
class MQueue
{
public:
	MQueue(int size=dfSize);
	~MQueue();
	bool Peek(int index,T *data) const;
	bool Pop();
	bool Push(T data);
	bool isEmpty() const { if (_rear == _front) return true; else return false; }
	bool isFull() const { if (_cnt>=_size)return true; else return false; }
	int getSize() const { return _size; }
	int getCnt() const { return _cnt; }
private:
	T *_data;
	int _size;
	int _front;
	int _rear;
	int _cnt;
};

template <class T>
MQueue<T>::MQueue(int size):_size(size),_front(0),_rear(0),_cnt(0)
{
	_data = new T[_size + 1];
}
template <class T>
MQueue<T>::~MQueue()
{
	delete[] _data;
}

template <class T>
bool MQueue<T>::Peek(int index, T *data) const
{
	if (index < _size)
	{
		*data = _data[(_front + index + 1) % _size];
		return true;
	}
	else
		return false;
}

template <class T>
bool MQueue<T>::Pop()
{
	if (!isEmpty())
	{
		_front = (_front + 1) % (_size + 1);
		_cnt--;
		return true;
	}
	else
		return false;
}

template <class T>
bool MQueue<T>::Push(T data)
{
	if (!isFull())
	{
		_rear = (_rear + 1) % (_size + 1);
		_data[_rear] = data;
		_cnt++;
		return true;
	}
	else
		return false;
}