#include <iostream>
#include <WinSock2.h>
#include <map>
#include "Packet.h"
#include "RingBuffer.h"
#include "Session.h"
#include "thread.h"
#include "define.h"
#include "network.h"

std::map<LONGLONG, Session *> sessionList;

SRWLOCK srwLock;


LONGLONG num = 0;

int in_check = 0;
int out_check = 0;

unsigned int WINAPI WorkerThread(LPVOID lpParam)
{
	int retVal;
	HANDLE hcp = (HANDLE)lpParam;

	Session *session;
	//LPOVERLAPPED lpOverlapped;
	MyOverlapped *pOverlapped;
	DWORD transferred;

	printf("Worker thread On\n");
	int test=10;
	while (1)
	{
		//이 두가지는 GetQueuedCompletionStatus가 실패하거나 하면 세팅되지 않음
		//이전 루프에서의 값을 보존하고 있음
		//이 경우 올바르지 않은 대상을 끊어낼 수 있는 문제가 생길 수 있음
		//일반적인 경우 초기화를 안해준다고 해서 문제가 생기진 않음
		//하지만 기능 변경 등의 문제로 문제가 발생할 위험 다소 있다
		transferred = 0;
		session = NULL;
		pOverlapped = NULL;
		

		int ret = GetQueuedCompletionStatus(hcp, &transferred, (PULONG_PTR)&session, (LPOVERLAPPED *)&pOverlapped, INFINITE);
		InterlockedAdd((LONG *)&session->GetIOCount(), 1);


		if (transferred == 0 && session == 0 && pOverlapped == 0)
		{
			InterlockedAdd((LONG *)&session->GetIOCount(), -1);
			//일반적으로 post를 통해 worker스레드를 종료시키는 목적으로 사용
			printf("000\n");
			break;
		}

		if (ret == false && pOverlapped == NULL)
		{
			InterlockedAdd((LONG *)&session->GetIOCount(), -1);

			printf("ret error\n");
			//iocp 오류
			continue;
		}

		if (transferred == 0)
		{
			//단 zero바이트를 send해서 동기적인 작동을 유도하는 기법을 사용할 경우
			//이 부분의 코드가 항상 끊김처리인 것은 아님
			//그러나 보통은 사용하지 않는 기법
			//끊김처리

			InterlockedExchange8((CHAR *)&session->GetSocketActive(), FALSE);
			closesocket(session->GetSocket());
			//InterlockedAdd((LONG *)&session->GetIOCount(), -1);
			printf("transffer 0 %d\n", session->GetSocket());
		}

		if (pOverlapped->type == TYPE::RECV)
		{
			InterlockedAdd((LONG *)&session->GetIOCount(), -1);
			session->GetRecvQ().MoveWritePos(transferred);
			session->GetRecvQ().UnLock();
			while(1)
			{
				if (CompleteRecvPacket(session) != SUCCESS)
					break;
			}
			session->RecvPost(TRUE);
			session->SendPost();
		}
		else if (pOverlapped->type == TYPE::SEND)
		{
			//SendFlag = false;
			InterlockedAdd((LONG *)&session->GetIOCount(), -1);
			session->GetSendQ().MoveReadPos(transferred);
			InterlockedExchange8(&session->GetSendFlag(), 1);
			session->SendPost();
		}
		
		//InterlockedAdd((LONG *)&session->GetIOCount(), -1);
		if (InterlockedAdd((LONG *)&session->GetIOCount(), -1) ==0)//&& !session->GetSocketActive())
		{
			InterlockedAdd((LONG *)&out_check, 1);
			printf("delete session %d\n", session->GetID());

			AcquireSRWLockExclusive(&srwLock);
			sessionList.erase(session->GetID());
			ReleaseSRWLockExclusive(&srwLock);

			delete session;
		}
	}


	return 0;
}

unsigned int WINAPI AcceptThread(LPVOID lpParam)
{
	HANDLE hcp = (HANDLE)lpParam;
	SOCKET listenSock = socket(AF_INET, SOCK_STREAM, 0);

	if (listenSock == INVALID_SOCKET)
	{
		printf("socket error\n");
		return -1;
	}

	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(PORT);
	int retval = bind(listenSock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));

	if (retval == SOCKET_ERROR)
	{
		printf("bind error\n");
		return -1;
	}

	retval = listen(listenSock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
	{
		printf("listen error\n");
		return -1;
	}

	Session *session;
	SOCKET sock;
	SOCKADDR_IN sockAddr;
	int addrLen;

	InitializeSRWLock(&srwLock);
	printf("Accept thread On\n");

	while (1)
	{
		addrLen = sizeof(sockAddr);
		sock = accept(listenSock, (SOCKADDR *)&(sockAddr), &addrLen);
		InterlockedAdd((LONG *)&in_check,1);
		if (sock == INVALID_SOCKET)
		{
			printf("accept error\n");
			continue;
		}

		int optval = 0;
		retval = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&optval, sizeof(optval));
		if (retval == SOCKET_ERROR)
		{
			printf("SO_RCVBUF error\n");
			return -1;
		}

		printf("client connect %d\n",num);

		session = new Session(sock, sockAddr,num);
		num++;
		

		CreateIoCompletionPort((HANDLE)sock, hcp, (ULONG_PTR)session, 0);

		//accept 순간에 성공하지 않으면 session이 생성되지 않은거나 다름이 없음
		if (session->RecvPost(FALSE))
		{
			AcquireSRWLockExclusive(&srwLock);
			sessionList.insert(std::make_pair(session->GetID(), session));
			ReleaseSRWLockExclusive(&srwLock);
		}
		else
			delete session;

		
	}

	return 0;
}