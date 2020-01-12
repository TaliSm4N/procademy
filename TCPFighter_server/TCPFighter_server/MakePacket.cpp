#include <Windows.h>

#include "Packet.h"
#include "RingBuffer.h"
#include "network.h"
#include "Protocol.h"
#include "MakePacket.h"

void MakePacketCreatePlayer(Session *session, Packet &p, DWORD id, BYTE dir, WORD x, WORD y, BYTE HP)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.byType = dfPACKET_SC_CREATE_MY_CHARACTER;
	header.bySize = 10;

	p.Clear();
	p.PutData((char *)&header, sizeof(header));
	p << id << dir << x << y << HP << dfNETWORK_PACKET_END;
}

void MakePacketMoveStart(Session *session, Packet &p, DWORD id, BYTE dir, WORD x, WORD y)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.byType = dfPACKET_SC_MOVE_START;
	header.bySize = 9;

	p.Clear();
	p.PutData((char *)&header, sizeof(header));
	p << id << dir << x << y  << dfNETWORK_PACKET_END;
}

void MakePacketMoveStop(Session *session, Packet &p, DWORD id, BYTE dir, WORD x, WORD y)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.byType = dfPACKET_SC_MOVE_STOP;
	header.bySize = 9;

	p.Clear();
	p.PutData((char *)&header, sizeof(header));
	p << id << dir << x << y << dfNETWORK_PACKET_END;
}

void MakePacketSync(Session *session, Packet &p, DWORD id, WORD x, WORD y)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.byType = dfPACKET_SC_SYNC;
	header.bySize = 8;

	p.Clear();
	p.PutData((char *)&header, sizeof(header));
	p << id << x << y << dfNETWORK_PACKET_END;
}