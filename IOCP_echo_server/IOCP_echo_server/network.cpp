#include <Windows.h>
#include "Packet.h"
#include "RingBuffer.h"
#include "Setting.h"
#include "protocol.h"
#include "network.h"

bool SendUnicast(Session* session, st_PACKET_HEADER& header, Packet& p)
{
	if (session == NULL)
		return false;

	if (session->sendQ.GetFreeSize() >= p.GetDataSize()+sizeof(header))
	{
		session->sendQ.Enqueue((char*)&header, sizeof(header));
		session->sendQ.Enqueue(p);
	}
	else
	{
		return false;
	}
}

bool SendUnicast(Session* session, Packet& p)
{
	if (session == NULL)
		return false;

	if (session->sendQ.GetFreeSize() >= p.GetDataSize())
	{
		session->sendQ.Enqueue(p);
	}
	else
	{
		return false;
	}
}