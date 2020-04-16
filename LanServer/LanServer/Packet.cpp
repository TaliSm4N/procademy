#define _WINSOCKAPI_
#include <Windows.h>

#include "header.h"
#include "MemoryPool.h"
#include "MemoryPoolTLS.h"
#include "Packet.h"

MemoryPoolTLS<Packet> *Packet::packetPool = NULL;

Packet::Packet()
	:mode(ERROR_MODE), err(E_NOERROR), front(0), rear(0), size(DEFAULT_PACKET_SIZE)
{
}

Packet::Packet(int iBufferSize)
	:mode(ERROR_MODE),err(E_NOERROR),front(0),rear(0),size(iBufferSize)
{
}

Packet::Packet(int iBufferSize,int Mode)
	: mode(Mode), err(E_NOERROR), front(0), rear(0),size(iBufferSize)
{
}

Packet::~Packet()
{
}

void Packet::Release(void)
{
	if (InterlockedDecrement((LONG *)&refCnt) == 0)
	{
		delete this;
	}
}
void Packet::Ref()
{
	InterlockedIncrement((LONG *)&refCnt);
}

bool Packet::UnRef(void)
{
	if (InterlockedDecrement((LONG *)&refCnt) == 0)
	{
		return true;
		//delete this;
	}

	return false;
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

	memcpy(GetBufferPtr()+rear, chpSrc, iSrcSize);
	rear += iSrcSize;

	return iSrcSize;
}

Packet &Packet::operator = (Packet &clSrcPacket)
{
	size = clSrcPacket.size;
	memcpy(buf, clSrcPacket.buf, clSrcPacket.size);
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
		memcpy(buf + rear, &byValue, sizeof(BYTE));
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
		memcpy(buf + rear, &chValue, sizeof(char));
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
		memcpy(buf + rear, &shValue, sizeof(short));
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
		memcpy(buf + rear, &wValue, sizeof(WORD));
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
		memcpy(buf + rear, &iValue, sizeof(int));
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
		memcpy(buf + rear, &dwValue, sizeof(DWORD));
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
		memcpy(buf + rear, &fValue, sizeof(float));
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
		memcpy(buf + rear, &iValue, sizeof(__int64));
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
		memcpy(buf + rear, &dValue, sizeof(double));
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
		memcpy(buf + rear, &iValue, sizeof(UINT64));
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
		memcpy(buf + rear, &iValue, sizeof(UINT));
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
		memcpy(&byValue, buf + front, sizeof(BYTE));
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
		memcpy(&chValue, buf + front, sizeof(char));
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
		memcpy(&shValue, buf + front, sizeof(short));
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
		memcpy(&iValue, buf + front, sizeof(int));
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
		memcpy(&dwValue, buf + front, sizeof(DWORD));
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
		memcpy(&fValue, buf + front, sizeof(float));
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
		memcpy(&iValue, buf + front, sizeof(__int64));
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
		memcpy(&dValue, buf + front, sizeof(double));
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
		memcpy(&iValue, buf + front, sizeof(UINT64));
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
		memcpy(&iValue, buf + front, sizeof(UINT));
		MoveReadPos(sizeof(UINT));
	}
	else
	{
		err = E_GETDATA_ERROR;
	}

	return *this;
}

void Packet::GetHeader(HEADER *desheader)
{
	//헤더 종류에 따라 코드 수정필요
	desheader->len = header.len;
}

void Packet::PutHeader(HEADER *srcheader)
{
	//헤더 종류에 따라 코드 수정필요
	header.len = srcheader->len;
}

void Packet::Init()
{
	packetPool = new MemoryPoolTLS<Packet>(10000, true);
}

Packet *Packet::Alloc()
{
	Packet *ret = packetPool->Alloc();
	ret->Ref();

	return ret;
}

bool Packet::Free(Packet *p)
{
	if (p->UnRef())
	{
		return packetPool->Free(p);
	}

	return true;
}