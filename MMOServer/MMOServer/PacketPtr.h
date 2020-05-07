#pragma once


class PacketPtr
{
public:
	PacketPtr();
	PacketPtr(const PacketPtr& tptr);
	~PacketPtr();

	PacketPtr &operator = (PacketPtr &clSrcPacket);

	//////////////////////////////////////////////////////////////////////////
	// 넣기. 각 변수 타입마다 모두 만듬.
	//////////////////////////////////////////////////////////////////////////
	PacketPtr &operator << (BYTE byValue);
	PacketPtr &operator << (char chValue);

	PacketPtr &operator << (short shValue);
	PacketPtr &operator << (WORD wValue);

	PacketPtr &operator << (int iValue);
	PacketPtr &operator << (DWORD dwValue);
	PacketPtr &operator << (float fValue);

	PacketPtr &operator << (__int64 iValue);
	PacketPtr &operator << (double dValue);
	PacketPtr &operator << (UINT64 iValue);
	PacketPtr &operator << (UINT iValue);


	//////////////////////////////////////////////////////////////////////////
	// 빼기. 각 변수 타입마다 모두 만듬.
	//////////////////////////////////////////////////////////////////////////
	PacketPtr &operator >> (BYTE &byValue);
	PacketPtr &operator >> (char &chValue);

	PacketPtr &operator >> (short &shValue);
	PacketPtr &operator >> (WORD &wValue);

	PacketPtr &operator >> (int &iValue);
	PacketPtr &operator >> (DWORD &dwValue);
	PacketPtr &operator >> (float &fValue);

	PacketPtr &operator >> (__int64 &iValue);
	PacketPtr &operator >> (double &dValue);
	PacketPtr &operator >> (UINT64 &iValue);
	PacketPtr &operator >> (UINT &iValue);


	Packet *GetPacket() { return p; }
private:
	Packet *p;
	int *refCnt;
};