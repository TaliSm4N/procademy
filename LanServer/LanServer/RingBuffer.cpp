#define _WINSOCKAPI_
#include <Windows.h>
#include <string>

#include "header.h"
#include "MemoryPool.h"
#include "MemoryPoolTLS.h"
#include "Packet.h"
#include "RingBuffer.h"

RingBuffer::RingBuffer()
	:_size(0), _front(0), _rear(0), _capacity(RINGBUF_DEFAULT_SIZE)
{
	_buf = new char[RINGBUF_DEFAULT_SIZE + 1];
	InitializeSRWLock(&srwLock);
}

RingBuffer::RingBuffer(int iBufferSize)
	:_size(0),_front(0),_rear(0),_capacity(iBufferSize)
{
	_buf = new char[iBufferSize+1];
	InitializeSRWLock(&srwLock);
}

RingBuffer::~RingBuffer()
{
	delete []_buf;
}

void RingBuffer::Resize(int size)
{
}

int RingBuffer::GetBufferSize() const
{
	return _capacity;
}

int RingBuffer::GetUseSize() const
{
	return (_rear - _front + _capacity + 1) % (_capacity + 1);
}

int RingBuffer::GetFreeSize() const
{
	return _capacity - GetUseSize();
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
}


int RingBuffer::Enqueue(char *chpData, int iSize)
{

	int temp = GetFreeSize();

	if (temp < iSize)
	{
		iSize = temp;
	}
	temp = DirectEnqueueSize();

	if (temp < 0)
	{
		return -1;
	}

	if (temp < iSize)
	{
		memcpy(_buf + _rear,chpData, temp);
		_rear += temp;
		_rear %= _capacity + 1;
		memcpy(_buf + _rear, chpData + temp, iSize - temp);
		_rear += iSize - temp;
		_rear %= _capacity + 1;
	}
	else
	{
		memcpy(_buf + _rear, chpData, iSize);
		_rear += iSize;
		_rear %= _capacity + 1;
	}

	return iSize;
}
#ifdef _XSTRING_
int RingBuffer::Enqueue(std::wstring *str, int iSize)
{
	int temp = GetFreeSize();
	if (temp < iSize)
	{
		iSize = temp;
	}
	temp = DirectEnqueueSize();
	if (temp < iSize)
	{
		str->copy((wchar_t *)(_buf + _rear), temp);
		_rear += temp;
		_rear %= _capacity + 1;
		str->copy((wchar_t *)(_buf + _rear), iSize - temp, temp);
		_rear += iSize - temp;
		_rear %= _capacity + 1;
	}
	else
	{
		str->copy((wchar_t *)(_buf + _rear), temp);
		_rear += iSize;
		_rear %= _capacity + 1;
	}

	return iSize;
}
#endif

#ifdef __LUMO_PACKET__
int  RingBuffer::Enqueue(Packet *p)
{
	int temp = GetFreeSize();
	int size= p->GetDataSize();
	if (temp < size)
	{
		size = temp;
	}

	if (0 >= size)
		return 0;

	temp = DirectEnqueueSize();
	if (temp < size)
	{
		memcpy(_buf + _rear, p->GetBufferPtr(), temp);
		_rear += temp;
		_rear %= _capacity + 1;
		memcpy(_buf + _rear, p->GetBufferPtr() + temp, size - temp);
		_rear += size - temp;
		_rear %= _capacity + 1;
	}
	else
	{
		memcpy(_buf + _rear, p->GetBufferPtr(), size);
		_rear += size;
		_rear %= _capacity + 1;
	}

	return size;
}

#endif
int RingBuffer::Dequeue(char *chpData, int iSize)
{
	int temp = GetUseSize();

	if (temp < iSize)
	{
		iSize = temp;
	}

	if (0 >= iSize)
		return 0;

	temp = DirectDequeueSize();
	if (temp < iSize)
	{
		memcpy(chpData, _buf + _front, temp);
		MoveFront(temp);
		memcpy(chpData+temp, _buf + _front, iSize - temp);
		MoveFront(iSize-temp);
	}
	else
	{
		memcpy(chpData, _buf + _front, iSize);
		MoveFront(iSize);
	}

	return iSize;
}
#ifdef __LUMO_PACKET__
int  RingBuffer::Dequeue(Packet *p,int iSize)
{
	int temp = GetUseSize();
	int freeSize = p->GetBufferSize()-p->GetDataSize();

	if (temp < iSize)
	{
		iSize = temp;
	}
	temp = DirectDequeueSize();
	if (temp < iSize)
	{
		p->PutData(_buf + _front, temp);
		MoveFront(temp);
		p->PutData(_buf + _front, iSize - temp);
		MoveFront(iSize - temp);
	}
	else
	{
		p->PutData(_buf + _front, iSize);
		MoveFront(iSize);
	}

	return iSize;
}

#endif
#ifdef _XSTRING_
int RingBuffer::Dequeue(std::wstring *str, int iSize)
{
	int temp = GetUseSize();
	str->clear();
	if (temp < iSize)
	{
		iSize = temp;
	}

	if (0 >= iSize)
		return 0;

	temp = DirectDequeueSize();
	if (temp < iSize)
	{
		str->resize(iSize);
		str->insert(0, (wchar_t *)(_buf + _front),0 ,temp/sizeof(WCHAR));
		MoveFront(temp);
		str->insert(temp / sizeof(WCHAR)+1, (wchar_t *)(_buf + _front),0 ,(iSize-temp) / sizeof(WCHAR));
		MoveFront(iSize - temp);
	}
	else
	{
		str->insert(0, (wchar_t *)(_buf + _front), iSize / sizeof(WCHAR));
		MoveFront(iSize);
	}

	return iSize;
}
#endif

int RingBuffer::Peek(char *chpData, int iSize)
{
	int temp = GetUseSize();

	if (temp < iSize)
	{
		iSize = temp;
	}

	if (0 >= iSize)
		return 0;

	temp = DirectDequeueSize();
	if (temp < iSize)
	{
		memcpy(chpData, _buf + _front, temp);
		memcpy(chpData + temp, _buf, iSize - temp);
	}
	else
	{
		memcpy(chpData, _buf + _front, iSize);
	}

	return iSize;

}


bool RingBuffer::MoveFront(int size)
{
	_front += size;
	_front %= _capacity + 1;
	return true;
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

char *RingBuffer::GetBufPtr() const
{
	return _buf;
}

void RingBuffer::Lock()
{
	AcquireSRWLockExclusive(&srwLock);
}

void RingBuffer::UnLock()
{
	ReleaseSRWLockExclusive(&srwLock);
}

void RingBuffer::Reset() 
{ 
	_front = 0; 
	_rear = 0; 
	InitializeSRWLock(&srwLock);
}