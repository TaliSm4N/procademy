#include <WinSock2.h>
#include <iostream>

#include "NetServerLib.h"

Session::Session(SOCKET s, SOCKADDR_IN &sAddr,DWORD id)
	:sock(s),sockAddr(sAddr),sendFlag(1),sessionID(id), disconnectFlag(false)
{
	ZeroMemory(&sendOverlap, sizeof(sendOverlap));
	ZeroMemory(&recvOverlap, sizeof(recvOverlap));
	sendOverlap.type = TYPE::SEND;
	recvOverlap.type = TYPE::RECV;
	sendQ = new LockFreeQueue<Packet *>(1000);
	//InitializeSRWLock(&sessionLock);
	IOBlock = new IOChecker;
	IOBlock->IOCount = 1;
	IOBlock->releaseFlag = 0;
	//_IOChecker.IOCount = 0;
	//_IOChecker.releaseFlag = false;
}

Session::Session()
	:sendFlag(1),sock(INVALID_SOCKET), disconnectFlag(false)
{
	ZeroMemory(&sendOverlap, sizeof(sendOverlap));
	ZeroMemory(&recvOverlap, sizeof(recvOverlap));
	sendOverlap.type = TYPE::SEND;
	recvOverlap.type = TYPE::RECV;
	sendQ = new LockFreeQueue<Packet *>(1000);
	IOBlock = new IOChecker;
	IOBlock->IOCount = 1;
	IOBlock->releaseFlag = 0;
	
	//IOBlock->IOCount = 0;
}

void Session::SetSessionInfo(SOCKET s, SOCKADDR_IN &sAddr, DWORD ID)
{
	sock = s;
	sockAddr = sAddr;
	sessionID = ID;
	//sendFlag = 1;
	//sockActive = FALSE;
	sendPacketCnt = 0;
	
	//IOBlock->releaseFlag = 0;
	//IOBlock->IOCount = 0;
	//
	//InterlockedExchange64(&IOBlock->IOCount, 0);
	
	ZeroMemory(&sendOverlap, sizeof(sendOverlap));
	ZeroMemory(&recvOverlap, sizeof(recvOverlap));
	sendOverlap.type = TYPE::SEND;
	recvOverlap.type = TYPE::RECV;
	disconnectFlag = false;
	//sendQ.Reset();
	recvQ.Reset();

	//InitializeSRWLock(&sessionLock);
}

Session::~Session()
{
	Release();
}

bool Session::Disconnect()
{
	InterlockedExchange8((char *)&disconnectFlag,true);
	//shutdown(sock, SD_BOTH);
	SOCKET closeSock = sock;
	if (InterlockedExchange(&sock, INVALID_SOCKET) != INVALID_SOCKET)
	{
		_closeSocket = closeSock;
		closesocket(closeSock);
	}
	
	return true;
}

bool Session::CloseSocket()
{
	//shutdown(sock, SD_BOTH);
	SOCKET closeSock = sock;
	if (InterlockedExchange(&sock, INVALID_SOCKET) != INVALID_SOCKET)
	{
		_closeSocket = closeSock;
		closesocket(closeSock);
	}

	return true;
}


BOOL Session::Release()
{
	Disconnect();

	delete IOBlock;
	delete sendQ;

	return TRUE;
}
