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
		//�� �ΰ����� GetQueuedCompletionStatus�� �����ϰų� �ϸ� ���õ��� ����
		//���� ���������� ���� �����ϰ� ����
		//�� ��� �ùٸ��� ���� ����� ��� �� �ִ� ������ ���� �� ����
		//�Ϲ����� ��� �ʱ�ȭ�� �����شٰ� �ؼ� ������ ������ ����
		//������ ��� ���� ���� ������ ������ �߻��� ���� �ټ� �ִ�
		transferred = 0;
		session = NULL;
		pOverlapped = NULL;
		

		int ret = GetQueuedCompletionStatus(hcp, &transferred, (PULONG_PTR)&session, (LPOVERLAPPED *)&pOverlapped, INFINITE);
		InterlockedAdd((LONG *)&session->GetIOCount(), 1);


		if (transferred == 0 && session == 0 && pOverlapped == 0)
		{
			InterlockedAdd((LONG *)&session->GetIOCount(), -1);
			//�Ϲ������� post�� ���� worker�����带 �����Ű�� �������� ���
			printf("000\n");
			break;
		}

		if (ret == false && pOverlapped == NULL)
		{
			InterlockedAdd((LONG *)&session->GetIOCount(), -1);

			printf("ret error\n");
			//iocp ����
			continue;
		}

		if (transferred == 0)
		{
			//�� zero����Ʈ�� send�ؼ� �������� �۵��� �����ϴ� ����� ����� ���
			//�� �κ��� �ڵ尡 �׻� ����ó���� ���� �ƴ�
			//�׷��� ������ ������� �ʴ� ���
			//����ó��

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

		//accept ������ �������� ������ session�� �������� �����ų� �ٸ��� ����
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