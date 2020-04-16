#include <iostream>

#include "LanServerLib.h"

Session::Session(SOCKET s, SOCKADDR_IN &sAddr,DWORD id)
	:sock(s),sockAddr(sAddr),sendFlag(1),IOCount(0), sockActive(FALSE),sessionID(id)
{
	ZeroMemory(&sendOverlap, sizeof(sendOverlap));
	ZeroMemory(&recvOverlap, sizeof(recvOverlap));
	sendOverlap.type = TYPE::SEND;
	recvOverlap.type = TYPE::RECV;
	sendQ = new LockFreeQueue<Packet *>(1000);
	InitializeSRWLock(&sessionLock);
}

Session::Session()
	:sendFlag(1), IOCount(0), sockActive(FALSE)
{
	sendQ = new LockFreeQueue<Packet *>(1000);
}

void Session::SetSessionInfo(SOCKET s, SOCKADDR_IN &sAddr, DWORD ID)
{
	sock = s;
	sockAddr = sAddr;
	sessionID = ID;
	sendFlag = 1;
	IOCount = 0;
	sockActive = FALSE;
	ZeroMemory(&sendOverlap, sizeof(sendOverlap));
	ZeroMemory(&recvOverlap, sizeof(recvOverlap));
	sendOverlap.type = TYPE::SEND;
	recvOverlap.type = TYPE::RECV;

	//sendQ.Reset();
	recvQ.Reset();

	InitializeSRWLock(&sessionLock);
}

Session::~Session()
{
	Release();
}

BOOL Session::Release()
{
	if (InterlockedExchange8((CHAR *)&sockActive, FALSE))
	{
		closesocket(sock);
	}

	return TRUE;
}

void Session::Lock()
{
	AcquireSRWLockExclusive(&sessionLock);
}
void Session::Unlock()
{
	ReleaseSRWLockExclusive(&sessionLock);
}