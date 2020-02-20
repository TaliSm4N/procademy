#include <Windows.h>
#include "Packet.h"
#include "RingBuffer.h"
#include "Session.h"
#include "network.h"
#include "define.h"
#include "PacketProc.h"

bool Echo(Session *session, Packet &p)
{
	Header header;

	header.len = 8;

	LONGLONG data;

	p >> data;

	p.Clear();
	p.PutData((char *)&header, sizeof(header));
	p << data;

	SendUnicast(session, p);

	return TRUE;
}