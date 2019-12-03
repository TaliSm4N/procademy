#include "stdafx.h"
#include "RingBuffer.h"
#include <iostream>

RingBuffer::RingBuffer(int iBufferSize)
	:_size(0),_front(0),_rear(0),_capacity(iBufferSize)
{
	_buf = new char[iBufferSize+1];
	//_buf = new char[iBufferSize+1];
}

void RingBuffer::Resize(int size)
{
}

int RingBuffer::GetBufferSize() const
{
	return _capacity;
	//return _capacity;
}

int RingBuffer::GetUseSize() const
{
	return (_rear - _front + _capacity + 1) % (_capacity + 1);
	//return (_rear - _front + _capacity) % (_capacity + 1);

	//return _size;
}

int RingBuffer::GetFreeSize() const
{
	return _capacity - GetUseSize();
	//return (_capacity - (_rear - _front + _capacity) % (_capacity + 1));
	//return _capacity - _size;
}

int RingBuffer::DirectEnqueueSize() const
{
	if (_front > _rear)
	{
		return _front - _rear - 1;
	}
	else
	{
		if (_front == 0)
		{
			return _capacity - _rear;
		}
		else
		{
			return _capacity + 1 - _rear;
		}
		
	}

	//return _capacity - _rear;
}

int RingBuffer::DirectDequeueSize() const
{

	if (_rear > _front)
	{
		return _rear - _front;
	}
	else
	{
		return _capacity + 1 - _front;
	}

	/*
	if (_capacity - _front<_size)
	{
		return _capacity - _front;
	}

	return _size;
	*/
}


int RingBuffer::Enqueue(char *chpData, int iSize)
{

	int temp = GetFreeSize();
	if (temp < iSize)
	{
		iSize = temp;
	}
	temp = DirectEnqueueSize();
	if (temp < iSize)
	{
		memcpy_s(_buf + _rear, temp,chpData, temp);
		_rear += temp;
		_rear %= _capacity + 1;
		memcpy_s(_buf + _rear, iSize - temp, chpData + temp, iSize - temp);
		_rear += iSize - temp;
		_rear %= _capacity + 1;
	}
	else
	{
		memcpy_s(_buf + _rear, iSize, chpData, iSize);
		_rear += iSize;
		_rear %= _capacity + 1;
	}

	return iSize;
	/*
	int e_size = DirectEnqueueSize();
	int len;

	if (GetFreeSize() < iSize)
	{
		iSize = GetFreeSize();
	}
	

	if (DirectEnqueueSize() > iSize)
	{
		len = iSize;
		memcpy_s(_buf + _rear, len, chpData, len);
	}
	else
	{
		len = DirectEnqueueSize();
		memcpy_s(_buf + _rear, len, chpData, len);
		iSize -= len;
		memcpy_s(_buf, iSize, chpData + len, iSize);
		iSize += len;
	}

	_rear += iSize;
	_rear %= _capacity + 1;

	_size += iSize;

	return iSize;
	*/
}

int RingBuffer::Dequeue(char *chpData, int iSize)
{
	int temp = GetUseSize();

	if (temp < iSize)
	{
		iSize = temp;
	}

	temp = DirectDequeueSize();
	if (temp < iSize)
	{
		memcpy_s(chpData, temp, _buf + _front, temp);
		MoveFront(temp);
		memcpy_s(chpData+temp, iSize-temp, _buf + _front, iSize - temp);
		MoveFront(iSize-temp);
	}
	else
	{
		memcpy_s(chpData, iSize, _buf + _front, iSize);
		MoveFront(iSize);
	}

	return iSize;
	/*
	int len;

	if (iSize > GetUseSize())
		iSize = GetUseSize();

	if (DirectDequeueSize() > iSize)
	{
		len = iSize;
		memcpy_s(chpData, len, _buf+_front, len);
	}
	else
	{
			len = DirectDequeueSize();
		memcpy_s(chpData, len, _buf+_front, len);
		iSize -= len;

		memcpy_s(chpData + len, iSize, _buf, iSize);
		iSize += len;
	}

	if (MoveFront(iSize))
	{
		return iSize;
	}
	else
	{
		return 0;
	}
	*/
}

int RingBuffer::Peek(char *chpData, int iSize)
{
	int temp = GetUseSize();

	if (temp < iSize)
	{
		iSize = temp;
	}

	temp = DirectDequeueSize();
	if (temp < iSize)
	{
		memcpy_s(chpData, temp, _buf + _front, temp);
		//MoveFront(temp);
		memcpy_s(chpData + temp, iSize - temp, _buf, iSize - temp);
		//MoveFront(iSize - temp);
	}
	else
	{
		memcpy_s(chpData, iSize, _buf + _front, iSize);
		//MoveFront(iSize);
	}

	return iSize;
	/*
	int len;

	if (iSize > GetUseSize())
		iSize = GetUseSize();

	if (DirectDequeueSize() > iSize)
	{
		len = iSize;
		memcpy_s(chpData, len, _buf + _front, len);
	}
	else
	{
		len = DirectDequeueSize();
		memcpy_s(chpData, len, _buf + _front, len);
		iSize -= len;

		memcpy_s(chpData + len, iSize, _buf, iSize);
		iSize += len;
	}
	_size -= iSize;
	return iSize;
	*/
}

bool RingBuffer::MoveFront(int size)
{
	_front += size;
	_front %= _capacity + 1;
	return true;
	/*
	if (_size < size)
	{
		return false;
	}
	
	_size -= size;
	_front += size;
	_front %= _capacity + 1;


	return true;
	*/
}

bool RingBuffer::MoveRear(int size)
{
	_rear += size;
	_rear %= _capacity + 1;
	return true;
}

char *RingBuffer::GetWritePos() const
{
	return _buf + _rear;
}

char *RingBuffer::GetReadPos() const
{
	return _buf + _front;
}