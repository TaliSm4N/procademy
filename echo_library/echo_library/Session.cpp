#include <WinSock2.h>
#include <iostream>
#include "Packet.h"
#include "RingBuffer.h"
#include "Session.h"

Session::Session(SOCKET s, SOCKADDR_IN &sAddr,LONGLONG id)
	:sock(s),sockAddr(sAddr),sendFlag(1),IOCount(0), sockActive(TRUE),sessionID(id)
{
	ZeroMemory(&sendOverlap, sizeof(sendOverlap));
	ZeroMemory(&recvOverlap, sizeof(recvOverlap));
	sendOverlap.type = TYPE::SEND;
	recvOverlap.type = TYPE::RECV;
}

BOOL Session::RecvPost(BOOL test)
{

	if (!sockActive)
		return false;
	recvQ.Lock();
	WSABUF wsabuf[2];
	wsabuf[0].len = recvQ.DirectEnqueueSize();
	wsabuf[0].buf = recvQ.GetWritePos();
	wsabuf[1].len = recvQ.GetFreeSize() - recvQ.DirectEnqueueSize();
	wsabuf[1].buf = recvQ.GetBufPtr();


	DWORD flags = 0;

	InterlockedAdd((LONG *)&IOCount, 1);
	int retval = WSARecv(sock, wsabuf, 2, NULL, &flags, (OVERLAPPED *)&recvOverlap, NULL);
	//printf("recv\n");
	if (retval == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		//if (WSAGetLastError() != ERROR_IO_PENDING)
		if(err!=ERROR_IO_PENDING)
		{
			printf("%d\n",err);
			if (!test)
				printf("-----");
			printf("Not Overlapped Recv I/O %d\n", sessionID);
			InterlockedAdd((LONG *)&IOCount, -1);
			//check delete

			return false;
		}
	}

	return true;
}

BOOL Session::SendPost()
{
	if (!sockActive)
		return false;

	if (sendQ.GetUseSize() <= 0)
		return false;

	if (InterlockedExchange8(&sendFlag, 0) == 0)
		return false;

	WSABUF wsabuf[2];
	wsabuf[0].len = sendQ.DirectDequeueSize();
	wsabuf[0].buf = sendQ.GetReadPos();
	wsabuf[1].len = sendQ.GetUseSize() - sendQ.DirectDequeueSize();
	wsabuf[1].buf = sendQ.GetBufPtr();


	DWORD flags = 0;

	InterlockedAdd((LONG *)&IOCount, 1);
	int retval = WSASend(sock, wsabuf, 2, NULL, flags, (OVERLAPPED *)&sendOverlap, NULL);
	//printf("send\n");
	if (retval == SOCKET_ERROR)
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			printf("Not Overlapped Send I/O %d\n",sessionID);
			InterlockedAdd((LONG *)&IOCount, -1);
			//check delete

			return false;
		}
	}

	return true;
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