#include <Windows.h>
#include <Windows.h>
#include <ctime>
#include <iostream>
#include "Packet.h"
#include "RingBuffer.h"
#include "network.h"
#include "Protocol.h"
#include "MakePacket.h"
#include "Log.h"

void MakePacketCreatePlayer(Packet &p, DWORD id, BYTE dir, WORD x, WORD y, BYTE HP)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.byType = dfPACKET_SC_CREATE_MY_CHARACTER;
	header.bySize = 10;

	p.Clear();
	p.PutData((char *)&header, sizeof(header));
	p << id << dir << x << y << HP << dfNETWORK_PACKET_END;
}

void MakePacketCreateOtherPlayer(Packet &p, DWORD id, BYTE dir, WORD x, WORD y, BYTE HP)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.byType = dfPACKET_SC_CREATE_OTHER_CHARACTER;
	header.bySize = 10;

	p.Clear();
	p.PutData((char *)&header, sizeof(header));
	p << id << dir << x << y << HP << dfNETWORK_PACKET_END;
}

void MakePacketMoveStart(Packet &p, DWORD id, BYTE dir, WORD x, WORD y)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.byType = dfPACKET_SC_MOVE_START;
	header.bySize = 9;

	p.Clear();
	p.PutData((char *)&header, sizeof(header));
	p << id << dir << x << y  << dfNETWORK_PACKET_END;
}

void MakePacketMoveStop(Packet &p, DWORD id, BYTE dir, WORD x, WORD y)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.byType = dfPACKET_SC_MOVE_STOP;
	header.bySize = 9;

	p.Clear();
	p.PutData((char *)&header, sizeof(header));
	p << id << dir << x << y << dfNETWORK_PACKET_END;
}

void MakePacketRemovePlayer(Packet &p, DWORD id)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.byType = dfPACKET_SC_DELETE_CHARACTER;
	header.bySize = 4;

	p.Clear();
	p.PutData((char *)&header, sizeof(header));
	p << id << dfNETWORK_PACKET_END;
}

void MakePacketSync(Packet &p, DWORD id, WORD x, WORD y)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.byType = dfPACKET_SC_SYNC;
	header.bySize = 8;

	p.Clear();
	p.PutData((char *)&header, sizeof(header));
	p << id << x << y << dfNETWORK_PACKET_END;
}

void MakePacketEcho(Packet &p, DWORD time)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.byType = dfPACKET_SC_ECHO;
	header.bySize = 4;

	p.Clear();
	p.PutData((char *)&header, sizeof(header));
	p << time << dfNETWORK_PACKET_END;
}

void MakePacketAttack1(Packet &p, DWORD id, BYTE direction, WORD x, WORD y)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.byType = dfPACKET_SC_ATTACK1;
	header.bySize = 9;

	p.Clear();
	p.PutData((char *)&header, sizeof(header));
	p << id << direction << x << y << dfNETWORK_PACKET_END;
}

void MakePacketAttack2(Packet &p, DWORD id, BYTE direction, WORD x, WORD y)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.byType = dfPACKET_SC_ATTACK2;
	header.bySize = 9;

	p.Clear();
	p.PutData((char *)&header, sizeof(header));
	p << id << direction << x << y << dfNETWORK_PACKET_END;
}

void MakePacketAttack3(Packet &p, DWORD id, BYTE direction, WORD x, WORD y)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.byType = dfPACKET_SC_ATTACK3;
	header.bySize = 9;

	p.Clear();
	p.PutData((char *)&header, sizeof(header));
	p << id << direction << x << y << dfNETWORK_PACKET_END;
}

void MakePacketDamage(Packet &p, DWORD attackID, DWORD damageID, BYTE DamageHP)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.byType = dfPACKET_SC_DAMAGE;
	header.bySize = 9;

	p.Clear();
	p.PutData((char *)&header, sizeof(header));
	p << attackID << damageID << DamageHP << dfNETWORK_PACKET_END;
}