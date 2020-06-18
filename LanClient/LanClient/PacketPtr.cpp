#include <Windows.h>

#include "header.h"
#include "MemoryPool.h"
#include "MemoryPoolTLS.h"
#include "Packet.h"
#include "PacketPtr.h"

PacketPtr::PacketPtr()
{
	p = new Packet;
	refCnt = new int;
	*refCnt = 1;
}

PacketPtr::PacketPtr(const PacketPtr& tptr)
{
	p = tptr.p;
	refCnt = tptr.refCnt;

	*refCnt++;
}

PacketPtr::~PacketPtr()
{
	(*refCnt)--;

	if (*refCnt == 0)
	{
		delete p;
		delete refCnt;
	}
}

PacketPtr &PacketPtr::operator = (PacketPtr &clSrcPacket)
{
	p = clSrcPacket.p;
	refCnt = clSrcPacket.refCnt;

	*refCnt++;

	return *this;
}

//////////////////////////////////////////////////////////////////////////
// 넣기. 각 변수 타입마다 모두 만듬.
//////////////////////////////////////////////////////////////////////////
PacketPtr &PacketPtr::operator << (BYTE byValue)
{
	*p << byValue;

	return *this;
}
PacketPtr &PacketPtr::operator << (char chValue)
{
	*p << chValue;

	return *this;
}

PacketPtr &PacketPtr::operator << (short shValue)
{
	*p << shValue;

	return *this;
}
PacketPtr &PacketPtr::operator << (WORD wValue)
{
	*p << wValue;

	return *this;
}

PacketPtr &PacketPtr::operator << (int iValue)
{
	*p << iValue;

	return *this;
}

PacketPtr &PacketPtr::operator << (DWORD dwValue)
{
	*p << dwValue;

	return *this;
}
PacketPtr &PacketPtr::operator << (float fValue)
{
	*p << fValue;

	return *this;
}

PacketPtr &PacketPtr::operator << (__int64 iValue)
{
	*p << iValue;

	return *this;
}
PacketPtr &PacketPtr::operator << (double dValue)
{
	*p << dValue;

	return *this;
}
PacketPtr &PacketPtr::operator << (UINT64 iValue)
{
	*p << iValue;

	return *this;
}
PacketPtr &PacketPtr::operator << (UINT iValue)
{
	*p << iValue;

	return *this;
}

//////////////////////////////////////////////////////////////////////////
// 빼기. 각 변수 타입마다 모두 만듬.
//////////////////////////////////////////////////////////////////////////
PacketPtr &PacketPtr::operator >> (BYTE &byValue)
{
	*p >> byValue;

	return *this;
}
PacketPtr &PacketPtr::operator >> (char &chValue)
{
	*p >> chValue;

	return *this;
}

PacketPtr &PacketPtr::operator >> (short &shValue)
{
	*p >> shValue;

	return *this;
}
PacketPtr &PacketPtr::operator >> (WORD &wValue)
{
	*p >> wValue;

	return *this;
}

PacketPtr &PacketPtr::operator >> (int &iValue)
{
	*p >> iValue;

	return *this;
}
PacketPtr &PacketPtr::operator >> (DWORD &dwValue)
{
	*p >> dwValue;

	return *this;
}
PacketPtr &PacketPtr::operator >> (float &fValue)
{
	*p >> fValue;

	return *this;
}

PacketPtr &PacketPtr::operator >> (__int64 &iValue)
{
	*p >> iValue;

	return *this;
}
PacketPtr &PacketPtr::operator >> (double &dValue)
{
	*p >> dValue;

	return *this;
}
PacketPtr &PacketPtr::operator >> (UINT64 &iValue)
{
	*p >> iValue;

	return *this;
}
PacketPtr &PacketPtr::operator >> (UINT &iValue)
{
	*p >> iValue;

	return *this;
}