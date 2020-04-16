#include <WinSock2.h>
#include <iostream>

#include "LanServerLib.h"

Session::Session(SOCKET s, SOCKADDR_IN &sAddr,DWORD id)
	:sock(s),sockAddr(sAddr),sendFlag(1), sockActive(FALSE),sessionID(id),IOCount(0)
{
	ZeroMemory(&sendOverlap, sizeof(sendOverlap));
	ZeroMemory(&recvOverlap, sizeof(recvOverlap));
	sendOverlap.type = TYPE::SEND;
	recvOverlap.type = TYPE::RECV;
	sendQ = new LockFreeQueue<Packet *>(1000);
	InitializeSRWLock(&sessionLock);
	//_IOChecker.IOCount = 0;
	//_IOChecker.releaseFlag = false;
}

Session::Session()
	:sendFlag(1), sockActive(FALSE), IOCount(0)
{
	sendQ = new LockFreeQueue<Packet *>(1000);
	//_IOChecker.IOCount = 0;
	//_IOChecker.releaseFlag = false;
}

void Session::SetSessionInfo(SOCKET s, SOCKADDR_IN &sAddr, DWORD ID)
{
	sock = s;
	sockAddr = sAddr;
	sessionID = ID;
	sendFlag = 1;
	sockActive = FALSE;
	
	IOCount = 0;
	//_IOChecker.IOCount = 0;
	//_IOChecker.releaseFlag = false;
	
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