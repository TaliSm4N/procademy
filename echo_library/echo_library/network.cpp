#pragma comment(lib, "ws2_32")
#include <iostream>
#include <WinSock2.h>
#include <process.h>
#include <map>
#include "Packet.h"
#include "RingBuffer.h"
#include "Session.h"
#include "define.h"
#include "thread.h"
#include "network.h"
#include "PacketProc.h"
#include "networkLib.h"

extern std::map<LONGLONG, Session *> sessionList;

BOOL initNetwork()
{
	HANDLE hcp;
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return false;

	hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, RUNNING_CNT);

	if (hcp == NULL) return false;

	HANDLE hThread;
	DWORD dwThreadID;

	hThread = (HANDLE)_beginthreadex(NULL, 0, AcceptThread, hcp, 0, (unsigned int *)&dwThreadID);

	for (int i = 0; i < THREAD_CNT; i++)
	{
		hThread = (HANDLE)_beginthreadex(NULL, 0, WorkerThread, hcp, 0, (unsigned int *)&dwThreadID);
		if (hThread == NULL) return false;
		CloseHandle(hThread);
	}
}

PROCRESULT CompleteRecvPacket(Session *session)
{
	Header header;
	int recvQSize = session->GetRecvQ().GetUseSize();

	Packet payload;

	if (sizeof(header) > recvQSize)
		return NONE;

	session->GetRecvQ().Peek((char *)&header, sizeof(Header));

	if (recvQSize < header.len + sizeof(header))
		return NONE;

	session->GetRecvQ().MoveReadPos(sizeof(Header));

	if (session->GetRecvQ().Dequeue(payload, header.len) != header.len)
		return FAIL;

	//if (!PacketProc(session, 0, payload))
		//return FAIL;
	OnRecv(session->GetID(), payload);

	return SUCCESS;
}

bool PacketProc(Session *session, BYTE type, Packet &p)
{
	switch (type)
	{
	case 0:
		//Echo(session, p);
		break;
	}

	return true;
}

bool SendUnicast(Session* session, Packet& p)
{
	if (session == NULL)
	{
		return false;
	}

	if (session->GetSendQ().GetFreeSize() >= p.GetDataSize())
	{
		session->GetSendQ().Enqueue(p);
	}
	else
	{
		//_LOG(dfLOG_LEVEL_ERROR, L"SendQ size Lack");
		return false;
	}

	//monitorUnit.MonitorSendPacket();

	return true;
}


bool SendPacket(LONGLONG sessionID, Packet& p)
{
	AcquireSRWLockExclusive(&sessionListLock);
	auto iter = sessionList.find(sessionID);
	
	
	
	Header header;
	//printf("%d %d - sendpacket\n", sessionID, (*iter).second->GetID());

	//if (sessionID != (*iter).second->GetID())
	//{
	//	system("pause");
	//}
	//if (session == (*sessionList.end()).second)
	if(iter==sessionList.end())
	{
		ReleaseSRWLockExclusive(&sessionListLock);
		return false;
	}
	Session *session = (*iter).second;//(*sessionList.find(sessionID)).second;
	session->Lock();
	ReleaseSRWLockExclusive(&sessionListLock);
	header.len = p.GetDataSize();

	if (session->GetSendQ().GetFreeSize() >= p.GetDataSize()+sizeof(header))
	{
		session->GetSendQ().Enqueue((char *)&header,sizeof(header));
		session->GetSendQ().Enqueue(p);
	}
	else
	{
		session->Unlock();
		//_LOG(dfLOG_LEVEL_ERROR, L"SendQ size Lack");
		return false;
	}
	
	//monitorUnit.MonitorSendPacket();
	session->Unlock();
	return true;
}