#define _WINSOCKAPI_
#include <Windows.h>
#include "header.h"
#include "MemoryPool.h"
#include "MemoryPoolTLS.h"
#include "Packet.h"
#include "Profiler.h"


MemoryPoolTLS<Packet> *Packet::packetPool = NULL;
int Packet::_key = 0;
int Packet::_code = 0;

Packet::Packet()
	:mode(ERROR_MODE), err(E_NOERROR), front(0), rear(0), size(DEFAULT_PACKET_SIZE),encodeFlag(false), encodeCount(0), refCnt(0)
{
}

Packet::Packet(int iBufferSize)
	: mode(ERROR_MODE), err(E_NOERROR), front(0), rear(0), size(iBufferSize), encodeFlag(false), encodeCount(0), refCnt(0)
{
}

Packet::Packet(int iBufferSize, int Mode)
	: mode(Mode), err(E_NOERROR), front(0), rear(0), size(iBufferSize), encodeFlag(false), encodeCount(0),refCnt(0)
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
	err = E_NOERROR;
	encodeFlag = false;
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
	if (front + iSize <= rear)
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
	int useSize = GetDataSize();
	if (useSize < iSize)
	{
		//useSize = iSize;
		err = E_GETDATA_ERROR;
		return -1;
	}

	memcpy_s(chpDest, iSize, GetBufferPtr(), iSize);

	MoveReadPos(iSize);

	return iSize;
}

int Packet::PutData(char *chpSrc, int iSrcSize)
{
	int freeSize = GetBufferSize() - rear;
	if (freeSize < iSrcSize)
	{
		err = E_PUTDATA_ERROR;
		return -1;
	}

	memcpy(GetBufferPtr() + rear, chpSrc, iSrcSize);
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
	memcpy(desheader, &header, sizeof(HEADER));
}

void Packet::PutHeader(HEADER *srcheader)
{
	//헤더 종류에 따라 코드 수정필요
	//header.len = srcheader->len;
	memcpy(&header, srcheader, sizeof(HEADER));
}

void Packet::Init(int key, int code)
{
	packetPool = new MemoryPoolTLS<Packet>(10000, true);
	_key = key;
	_code = code;
}

Packet *Packet::Alloc()
{
	//PRO_BEGIN(L"PACKET_ALLOC");
	Packet *ret = packetPool->Alloc();
	ret->AllocTime = GetTickCount();
	ret->Ref();

	//PRO_END(L"PACKET_ALLOC");

	return ret;
}


bool Packet::Free(Packet *p)
{
	bool ret;
	//PRO_BEGIN(L"PACKET_FREE");
	if (p->UnRef())
	{
		ret = packetPool->Free(p);
		//PRO_END(L"PACKET_FREE");

		return ret;
	}
	//PRO_END(L"PACKET_FREE");


	return true;
}

void Packet::encode()
{
	if (InterlockedExchange8((char *)&encodeFlag, true) == true)
		return;

	//PRO_BEGIN(L"ENCODE");

	InterlockedIncrement((LONG *)&encodeCount);
	header.code = Packet::GetCode();
	header.len = GetDataSize();
	header.RandKey = (BYTE)rand() % 256;
	header.CheckSum = 0;
	for (int i = 0; i < header.len; i++)
	{
		header.CheckSum += *(GetBufferPtr() + i)%256;
	}
	header.CheckSum %= 256;

	char p = 0;

	p = header.CheckSum ^ (header.RandKey + p + 1);
	header.CheckSum = p ^ (_key + 1);

	for (int i = 0; i <= header.len; i++)
	{
		p = buf[i] ^ (header.RandKey + p + i + 2);
		buf[i] = p ^ (_key + i + 2 + buf[i - 1]);
	}

	//PRO_END(L"ENCODE");
}

void Packet::decode()
{
	if (InterlockedExchange8((char *)&encodeFlag, false) == false)
		return;

	char p = 0;
	char before_p = 0;
	char before_e = 0;

	p = header.CheckSum ^ (_key + 1);
	before_p = p;
	before_e = header.CheckSum;
	header.CheckSum = p ^ (header.RandKey + 1);

	for (int i = 0; i <= header.len; i++)
	{
		before_p = p;
		p = buf[i] ^ (before_e + _key + i + 2);
		before_e = buf[i];
		buf[i] = p ^ (before_p + header.RandKey + i + 2);
	}
}

bool Packet::VerifyCheckSum()
{
	BYTE check = 0x0;

	for (int i = 0; i < header.len; i++)
	{
		check += buf[i] % 256;
	}
	check %= 256;

	if (check != header.CheckSum)
	{
		return false;
	}

	return true;
}