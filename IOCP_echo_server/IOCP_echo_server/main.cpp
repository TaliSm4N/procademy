#pragma comment(lib, "ws2_32")
#include <iostream>
#include <tchar.h>
#include <WinSock2.h>
#include <process.h>
#include <list>
#include "Packet.h"
#include "RingBuffer.h"
#include "Setting.h"
#include "PacketProc.h"
#include "protocol.h"
#include "network.h"

BOOL init();
unsigned int WINAPI WorkerThread(LPVOID lpParam);
void Accept();
bool procRecv(Session *session);
bool recvStress(Session *client, Packet &p);


HANDLE hcp;

std::list<Session *> session_list;

int _tmain(int argc, _TCHAR* argv[])
{
	if (init())
	{
		Accept();
	}

	printf("End\n");

	return 0;
}

BOOL init()
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return false;

	hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, RUNNING_CNT);

	if (hcp == NULL) return false;

	HANDLE hThread;
	DWORD dwThreadID;
	for (int i = 0; i < THREAD_CNT; i++)
	{
		hThread = (HANDLE)_beginthreadex(NULL, 0, WorkerThread, hcp, 0, (unsigned int *)&dwThreadID);
		if (hThread == NULL) return false;
		CloseHandle(hThread);
	}
}

void Accept()
{
	SOCKET listenSock = socket(AF_INET, SOCK_STREAM, 0);

	if (listenSock == INVALID_SOCKET)
	{
		printf("socket error\n");
		return;
	}

	


	//int optlen = sizeof(optval);
	//getsockopt(listenSock, SOL_SOCKET, SO_RCVBUF, (char *)&optval, &optlen);
	//printf("%d\n",optval);


	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(PORT);
	int retval = bind(listenSock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));

	if (retval == SOCKET_ERROR)
	{
		printf("bind error\n");
		return;
	}

	retval = listen(listenSock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
	{
		printf("listen error\n");
		return;
	}

	Session *session;
	SOCKET sock;
	SOCKADDR_IN sockAddr;
	int addrLen;
	while (1)
	{
		
		addrLen = sizeof(sockAddr);
		sock = accept(listenSock, (SOCKADDR *)&(sockAddr), &addrLen);

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
			return;
		}

		printf("client connect\n");

		CreateIoCompletionPort((HANDLE)sock, hcp, sock, 0);

		session = new Session();
		session->sock = sock;
		session->sockAddr = sockAddr;
		session->sendFlag = FALSE;
		ZeroMemory(&(session->sendOverlap), sizeof(session->sendOverlap));
		ZeroMemory(&(session->recvOverlap), sizeof(session->recvOverlap));
		session->sendOverlap.type = TYPE::SEND;
		session->recvOverlap.type = TYPE::RECV;
		session->recvOverlap.session = session;
		session->sendOverlap.session = session;

		session_list.push_back(session);

		WSABUF wsabuf[2];
		wsabuf[0].len = session->recvQ.DirectEnqueueSize();
		wsabuf[0].buf = session->recvQ.GetWritePos();
		wsabuf[1].len=session->recvQ.GetFreeSize()- session->recvQ.DirectEnqueueSize();
		wsabuf[1].buf = session->recvQ.GetBufPtr();

		
		DWORD flags = 0;
		retval = WSARecv(session->sock, wsabuf, 2, NULL, &flags, (OVERLAPPED *)&(session->recvOverlap), NULL);

		if (retval == SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
			{
				printf("Not Overlapped Recv I/O\n");
			}
		}
	}

	WSACleanup();
}

unsigned int WINAPI WorkerThread(LPVOID lpParam)
{
	int retVal;
	HANDLE hcp = (HANDLE)lpParam;

	DWORD lpNumberOfBytesTransferred;
	ULONG_PTR lpCompletionKey;
	LPOVERLAPPED lpOverlapped;

	printf("Worker thread On\n");

	while (1)
	{
		//이 두가지는 GetQueuedCompletionStatus가 실패하거나 하면 세팅되지 않음
		//이전 루프에서의 값을 보존하고 있음
		//이 경우 올바르지 않은 대상을 끊어낼 수 있는 문제가 생길 수 있음
		//일반적인 경우 초기화를 안해준다고 해서 문제가 생기진 않음
		//하지만 기능 변경 등의 문제로 문제가 발생할 위험 다소 있다
		lpNumberOfBytesTransferred = 0;
		lpCompletionKey = NULL;
		lpOverlapped = NULL;

		retVal = GetQueuedCompletionStatus(hcp, &lpNumberOfBytesTransferred, &lpCompletionKey, &lpOverlapped, INFINITE);

		if (lpNumberOfBytesTransferred == 0 && lpCompletionKey == 0 && lpOverlapped == 0)
		{
			//일반적으로 post를 통해 worker스레드를 종료시키는 목적으로 사용
			printf("worker thread die\n");
			return -1;
		}

		if (retVal == false && lpOverlapped == NULL)
		{
			//iocp 오류
			printf("iocp error\n");
			return -1;
		}

		if (lpNumberOfBytesTransferred == 0)
		{
			//단 zero바이트를 send해서 동기적인 작동을 유도하는 기법을 사용할 경우
			//이 부분의 코드가 항상 끊김처리인 것은 아님
			//그러나 보통은 사용하지 않는 기법
			//끊김처리
		}
		else if (((MyOverlapped *)lpOverlapped)->type == TYPE::RECV)
		{
			//printf("recv\n");
			Session *session = (Session *)((MyOverlapped *)lpOverlapped)->session;
			session->recvQ.MoveWritePos(lpNumberOfBytesTransferred);

			
				//recvRingbuffer Message 완성 판단
				//Recv Packet Proc
			//procRecv(session);
			

			Packet p;
			
			//proc
			session->recvQ.Dequeue(p, session->recvQ.GetUseSize());
			SendUnicast(session, p);
			
			if (!session->sendFlag)
			{
				DWORD sendFlags = 0;
				WSABUF sendWSAbuf[2];
				sendWSAbuf[0].len = session->sendQ.DirectDequeueSize();
				sendWSAbuf[0].buf = session->sendQ.GetReadPos();
				sendWSAbuf[1].len = session->sendQ.GetUseSize() - session->sendQ.DirectDequeueSize();
				sendWSAbuf[1].buf = session->sendQ.GetBufPtr();

				//DWORD test;
				session->sendFlag = TRUE;
				int retval = WSASend(session->sock, sendWSAbuf, 2, NULL, sendFlags, (OVERLAPPED *)&(session->sendOverlap), NULL);
				if (retval == SOCKET_ERROR)
				{
					int err;
					err = WSAGetLastError();
					printf("errno %d\n", err);
					//if (WSAGetLastError() != ERROR_IO_PENDING)
					if (err != ERROR_IO_PENDING)
					{
						printf("Not Overlapped Send I/O\n");
					}
				}
			}

			
			//send;;;

				//WSARecv 등록

			WSABUF recvWSAbuf[2];
			recvWSAbuf[0].len = session->recvQ.DirectEnqueueSize();
			recvWSAbuf[0].buf = session->recvQ.GetWritePos();
			recvWSAbuf[1].len = session->recvQ.GetFreeSize() - session->recvQ.DirectEnqueueSize();
			recvWSAbuf[1].buf = session->recvQ.GetBufPtr();

			DWORD recvFlags = 0;
			int retval = WSARecv(session->sock, recvWSAbuf, 2, NULL, &recvFlags, (OVERLAPPED *)&(session->recvOverlap), NULL);

			if (retval == SOCKET_ERROR)
			{
				if (WSAGetLastError() != ERROR_IO_PENDING)
				{
					printf("Not Overlapped Recv I/O \n");
				}
			}
		}
		else if (((MyOverlapped *)lpOverlapped)->type == TYPE::SEND)
		{
			Session *session = (Session *)((MyOverlapped *)lpOverlapped)->session;
			//printf("send\n");

			session->sendQ.MoveReadPos(lpNumberOfBytesTransferred);

			session->sendFlag = false;
		}
	}

	return 0;
}

bool procRecv(Session *session)
{
	//st_PACKET_HEADER header;
	//LoginUser* client = FindClient(userID);
	
	int size;
	int t_size;
	Packet payLoad;
	if (session == NULL)
		return false;

	while (1)
	{
		//if (sizeof(header) > session->recvQ.GetUseSize())
		//	break;
		//if (session->recvQ.Peek((char*)&header, sizeof(header)) < sizeof(header))
		//	break;

		//if (header.byCode != dfPACKET_CODE)
		//	return false;

		//if (session->recvQ.GetUseSize() < header.wPayloadSize + sizeof(header))
		//	break;


		//session->recvQ.MoveFront(sizeof(header));
		//session->recvQ.Dequeue(payLoad, header.wPayloadSize);

		//std::wcout << L"PacketRecv [UserNO:" << client->clientNo << L"] [Type:" << header.wMsgType << "]" << std::endl;
		//recvCount++;
		//switch (header.wMsgType)
		//{
		//case df_REQ_STRESS_ECHO:
			recvStress(session, payLoad);
		//	break;
		//default:
		//	return false;
		//	break;
		//}
	}

	return true;
}

bool recvStress(Session *client, Packet &p)
{
	//sendStress(client, p);

	st_PACKET_HEADER header;
	Packet payLoad;
	//header.byCode = dfPACKET_CODE;
	//header.wMsgType = df_RES_STRESS_ECHO;
	//header.wPayloadSize = payLoad.GetDataSize();

	SendUnicast(client, header, payLoad);

	return true;
}