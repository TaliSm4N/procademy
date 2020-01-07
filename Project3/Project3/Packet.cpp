#include "Packet.h"

Packet::Packet()
	:mode(ERROR_MODE), err(E_NOERROR), front(0), rear(0), size(DEFAULT_PACKET_SIZE)
{
	buf = new char[DEFAULT_PACKET_SIZE];
}

Packet::Packet(int iBufferSize)
	:mode(ERROR_MODE),err(E_NOERROR),front(0),rear(0),size(iBufferSize)
{
	buf = new char[iBufferSize];
}

Packet::Packet(int iBufferSize,int Mode)
	: mode(Mode), err(E_NOERROR), front(0), rear(0),size(iBufferSize)
{
	buf = new char[iBufferSize];
}

Packet::~Packet()
{
	delete buf;
}

void Packet::Release(void)
{
	front = 0;
	rear = 0;
}

void Packet::Clear()
{
	front = 0;
	rear = 0;
}

int Packet::MoveWritePos(int iSize)
{
	if (rear + iSize < size)
		rear += iSize;
	else
	{
		iSize = size - rear - 1;
		rear = size - 1;
	}

	return iSize;
}
int Packet::MoveReadPos(int iSize)
{
	if(front+iSize<=rear)
		front += iSize;
	else
	{
		iSize = rear - front;
		front = rear;
	}

	return iSize;
}

int Packet::GetData(char *chpDest, int iSize)
{
	int useSize=GetDataSize();
	if (useSize >= iSize)
		useSize = iSize;

	memcpy_s(chpDest, iSize, GetBufferPtr(), useSize);

	MoveReadPos(useSize);

	return useSize;
}

int Packet::PutData(char *chpSrc, int iSrcSize)
{
	int freeSize = GetBufferSize() - rear;
	if (freeSize < iSrcSize)
	{
		err = E_PUTDATA_ERROR;
		return -1;
	}

	memcpy_s(GetBufferPtr()+rear, freeSize, chpSrc, iSrcSize);
	rear += iSrcSize;

	return iSrcSize;
}

Packet &Packet::operator = (Packet &clSrcPacket)
{
	buf = new char[clSrcPacket.size];
	size = clSrcPacket.size;
	memcpy_s(buf, size, clSrcPacket.buf, clSrcPacket.size);
	err = clSrcPacket.err;
	mode = clSrcPacket.mode;
	front = clSrcPacket.front;
	rear = clSrcPacket.rear;

	return *this;
}

//////////////////////////////////////////////////////////////////////////
// 넣기. 각 변수 타입마다 모두 만듬.
//////////////////////////////////////////////////////////////////////////
Packet &Packet::operator << (BYTE byValue)
{
	if (rear + sizeof(BYTE) < size)
	{
		memcpy_s(buf + rear, sizeof(BYTE), &byValue, sizeof(BYTE));
		MoveWritePos(sizeof(BYTE));
	}
	else
	{
		err = E_PUTDATA_ERROR;
	}

	return *this;
}
Packet &Packet::operator << (char chValue)
{
	if (rear + sizeof(char) < size)
	{
		memcpy_s(buf + rear, sizeof(char), &chValue, sizeof(char));
		MoveWritePos(sizeof(char));
	}
	else
	{
		err = E_PUTDATA_ERROR;
	}

	return *this;
}

Packet &Packet::operator << (short shValue)
{
	if (rear + sizeof(short) < size)
	{
		memcpy_s(buf + rear, sizeof(short), &shValue, sizeof(short));
		MoveWritePos(sizeof(short));
	}
	else
	{
		err = E_PUTDATA_ERROR;
	}

	return *this;
}
Packet &Packet::operator << (WORD wValue)
{
	if (rear + sizeof(WORD) < size)
	{
		memcpy_s(buf + rear, sizeof(WORD), &wValue, sizeof(WORD));
		MoveWritePos(sizeof(WORD));
	}
	else
	{
		err = E_PUTDATA_ERROR;
	}

	return *this;
}

Packet &Packet::operator << (int iValue)
{
	if (rear + sizeof(int) < size)
	{
		memcpy_s(buf + rear, sizeof(int), &iValue, sizeof(int));
		MoveWritePos(sizeof(int));
	}
	else
	{
		err = E_PUTDATA_ERROR;
	}

	return *this;
}
Packet &Packet::operator << (DWORD dwValue)
{
	if (rear + sizeof(DWORD) < size)
	{
		memcpy_s(buf + rear, sizeof(DWORD), &dwValue, sizeof(DWORD));
		MoveWritePos(sizeof(DWORD));
	}
	else
	{
		err = E_PUTDATA_ERROR;
	}

	return *this;
}
Packet &Packet::operator << (float fValue)
{
	if (rear + sizeof(float) < size)
	{
		memcpy_s(buf + rear, sizeof(float), &fValue, sizeof(float));
		MoveWritePos(sizeof(float));
	}
	else
	{
		err = E_PUTDATA_ERROR;
	}

	return *this;
}

Packet &Packet::operator << (__int64 iValue)
{
	if (rear + sizeof(__int64) < size)
	{
		memcpy_s(buf + rear, sizeof(__int64), &iValue, sizeof(__int64));
		MoveWritePos(sizeof(__int64));
	}
	else
	{
		err = E_PUTDATA_ERROR;
	}

	return *this;
}
Packet &Packet::operator << (double dValue)
{
	if (rear + sizeof(double) < size)
	{
		memcpy_s(buf + rear, sizeof(double), &dValue, sizeof(double));
		MoveWritePos(sizeof(double));
	}
	else
	{
		err = E_PUTDATA_ERROR;
	}

	return *this;
}

Packet &Packet::operator << (UINT64 iValue)
{
	if (rear + sizeof(UINT64) < size)
	{
		memcpy_s(buf + rear, sizeof(UINT64), &iValue, sizeof(UINT64));
		MoveWritePos(sizeof(UINT64));
	}
	else
	{
		err = E_PUTDATA_ERROR;
	}

	return *this;
}

Packet &Packet::operator << (UINT iValue)
{
	if (rear + sizeof(UINT) < size)
	{
		memcpy_s(buf + rear, sizeof(UINT), &iValue, sizeof(UINT));
		MoveWritePos(sizeof(UINT));
	}
	else
	{
		err = E_PUTDATA_ERROR;
	}

	return *this;
}


//////////////////////////////////////////////////////////////////////////
// 빼기. 각 변수 타입마다 모두 만듬.
//////////////////////////////////////////////////////////////////////////
Packet &Packet::operator >> (BYTE &byValue)
{
	if (front + sizeof(BYTE) <= rear)
	{
		memcpy_s(&byValue, sizeof(BYTE), buf + front, sizeof(BYTE));
		MoveReadPos(sizeof(BYTE));
	}
	else
	{
		err = E_GETDATA_ERROR;
	}

	return *this;
}
Packet &Packet::operator >> (char &chValue)
{
	if (front + sizeof(char) <= rear)
	{
		memcpy_s(&chValue, sizeof(char), buf + front, sizeof(char));
		MoveReadPos(sizeof(char));
	}
	else
	{
		err = E_GETDATA_ERROR;
	}

	return *this;
}

Packet &Packet::operator >> (short &shValue)
{
	if (front + sizeof(short) <= rear)
	{
		memcpy_s(&shValue, sizeof(short), buf + front, sizeof(short));
		MoveReadPos(sizeof(short));
	}
	else
	{
		err = E_GETDATA_ERROR;
	}

	return *this;
}
Packet &Packet::operator >> (WORD &wValue)
{
	if (front + sizeof(WORD) <= rear)
	{
		memcpy_s(&wValue, sizeof(WORD), buf + front, sizeof(WORD));
		MoveReadPos(sizeof(WORD));
	}
	else
	{
		err = E_GETDATA_ERROR;
	}

	return *this;
}

Packet &Packet::operator >> (int &iValue)
{
	if (front + sizeof(int) <= rear)
	{
		memcpy_s(&iValue, sizeof(int), buf + front, sizeof(int));
		MoveReadPos(sizeof(int));
	}
	else
	{
		err = E_GETDATA_ERROR;
	}

	return *this;
}
Packet &Packet::operator >> (DWORD &dwValue)
{
	if (front + sizeof(DWORD) <= rear)
	{
		memcpy_s(&dwValue, sizeof(DWORD), buf + front, sizeof(DWORD));
		MoveReadPos(sizeof(DWORD));
	}
	else
	{
		err = E_GETDATA_ERROR;
	}

	return *this;
}
Packet &Packet::operator >> (float &fValue)
{
	if (front + sizeof(float) <= rear)
	{
		memcpy_s(&fValue, sizeof(float), buf + front, sizeof(float));
		MoveReadPos(sizeof(float));
	}
	else
	{
		err = E_GETDATA_ERROR;
	}

	return *this;
}

Packet &Packet::operator >> (__int64 &iValue)
{
	if (front + sizeof(__int64) <= rear)
	{
		memcpy_s(&iValue, sizeof(__int64), buf + front, sizeof(__int64));
		MoveReadPos(sizeof(__int64));
	}
	else
	{
		err = E_GETDATA_ERROR;
	}

	return *this;
}
Packet &Packet::operator >> (double &dValue)
{
	if (front + sizeof(double) <= rear)
	{
		memcpy_s(&dValue, sizeof(double), buf + front, sizeof(double));
		MoveReadPos(sizeof(double));
	}
	else
	{
		err = E_GETDATA_ERROR;
	}

	return *this;
}

Packet &Packet::operator >> (UINT64 &iValue)
{
	if (front + sizeof(UINT64) <= rear)
	{
		memcpy_s(&iValue, sizeof(UINT64), buf + front, sizeof(UINT64));
		MoveReadPos(sizeof(UINT64));
	}
	else
	{
		err = E_GETDATA_ERROR;
	}

	return *this;
}

Packet &Packet::operator >> (UINT &iValue)
{
	if (front + sizeof(UINT) <= rear)
	{
		memcpy_s(&iValue, sizeof(UINT), buf + front, sizeof(UINT));
		MoveReadPos(sizeof(UINT));
	}
	else
	{
		err = E_GETDATA_ERROR;
	}

	return *this;
}