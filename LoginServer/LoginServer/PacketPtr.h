#pragma once


class PacketPtr
{
public:
	PacketPtr();
	PacketPtr(const PacketPtr& tptr);
	~PacketPtr();

	PacketPtr &operator = (PacketPtr &clSrcPacket);

	//////////////////////////////////////////////////////////////////////////
	// �ֱ�. �� ���� Ÿ�Ը��� ��� ����.
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
	// ����. �� ���� Ÿ�Ը��� ��� ����.
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