#include <iostream>

#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <process.h>
#include <cstring>
#include <map>
#include "Packet.h"
#include "RingBuffer.h"
#include "Session.h"
#include "LanServer.h"

CLanServer::CLanServer()
	:_sessionCount(0)
{
	
}

bool CLanServer::Start(int port,int workerCnt,bool nagle,int maxUser)
{
	_port = port;
	_workerCnt = workerCnt;
	_nagle = nagle;
	_maxUser = maxUser;

	if (WSAStartup(MAKEWORD(2, 2), &_wsa) != 0) return false;

	_hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	if (_hcp == NULL) return false;

	InitializeSRWLock(&sessionListLock);

	_hAcceptThread = (HANDLE)_beginthreadex(NULL, 0, AcceptThread, this, 0, (unsigned int *)&_dwAcceptThreadID);

	_hWokerThreads = new HANDLE[_workerCnt];
	_dwWOrkerThreadIDs = new DWORD[_workerCnt];

	for (int i = 0; i < _workerCnt; i++)
	{
		_hWokerThreads[i]= (HANDLE)_beginthreadex(NULL, 0, WorkerThread, this, 0, (unsigned int *)&_dwWOrkerThreadIDs[i]);

		if (_hWokerThreads[i] == NULL) return false;
	}
}

unsigned int WINAPI CLanServer::AcceptThread(LPVOID lpParam)
{

	//HANDLE hcp = (HANDLE)lpParam;
	SOCKET listenSock = socket(AF_INET, SOCK_STREAM, 0);

	CLanServer *_this = (CLanServer *)lpParam;

	if (listenSock == INVALID_SOCKET)
	{
		//printf("socket error\n");
		return -1;
	}

	ZeroMemory(&_this->_sockAddr, sizeof(_this->_sockAddr));
	_this->_sockAddr.sin_family = AF_INET;
	_this->_sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	_this->_sockAddr.sin_port = htons(_this->_port);
	int retval = bind(listenSock, (SOCKADDR *)&_this->_sockAddr, sizeof(_this->_sockAddr));

	if (retval == SOCKET_ERROR)
	{
		//printf("bind error\n");
		return -1;
	}

	retval = listen(listenSock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
	{
		//printf("listen error\n");
		return -1;
	}

	Session *session;
	SOCKET sock;
	SOCKADDR_IN sockAddr;
	int addrLen;


	printf("Accept thread On\n");

	while (1)
	{
		addrLen = sizeof(sockAddr);
		sock = accept(listenSock, (SOCKADDR *)&(sockAddr), &addrLen);
		//InterlockedAdd((LONG *)&in_check, 1);
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

		//_this->OnConnectionRequest()


		printf("client connect %d\n", _this->GetSessionCount());

		session = new Session(sock, sockAddr, _this->_sessionCount);

		_this->_sessionCount++;

		CreateIoCompletionPort((HANDLE)sock, _this->_hcp, (ULONG_PTR)session, 0);

		AcquireSRWLockExclusive(&_this->sessionListLock);
		_this->sessionList.insert(std::make_pair(session->GetID(), session));
		ReleaseSRWLockExclusive(&_this->sessionListLock);
		_this->OnClientJoin(session->GetID());

		//accept ������ �������� ������ session�� �������� �����ų� �ٸ��� ����
		if (!session->RecvPost(FALSE))
		{
			AcquireSRWLockExclusive(&_this->sessionListLock);
			_this->sessionList.erase(session->GetID());
			session->Lock();
			ReleaseSRWLockExclusive(&_this->sessionListLock);

			_this->OnClientLeave(session->GetID());
			session->Unlock();
			delete session;
		}


	}

	return 0;
}

unsigned int WINAPI CLanServer::WorkerThread(LPVOID lpParam)
{
	int retVal;
	CLanServer *_this = (CLanServer *)lpParam;

	Session *session;
	//LPOVERLAPPED lpOverlapped;
	MyOverlapped *pOverlapped;
	DWORD transferred;

	printf("Worker thread On\n");
	int test = 10;
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


		int ret = GetQueuedCompletionStatus(_this->_hcp, &transferred, (PULONG_PTR)&session, (LPOVERLAPPED *)&pOverlapped, INFINITE);
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
			while (1)
			{
				if (_this->CompleteRecvPacket(session) != SUCCESS)
					break;
				//printf("success\n");
			}
			session->SendPost();
			////printf("recv->send\n");
			session->RecvPost(TRUE);
		}
		else if (pOverlapped->type == TYPE::SEND)
		{
			//SendFlag = false;
			InterlockedAdd((LONG *)&session->GetIOCount(), -1);
			session->GetSendQ().MoveReadPos(transferred);
			InterlockedExchange8(&session->GetSendFlag(), 1);
			session->SendPost();

			_this->OnSend(session->GetID(),transferred);
			//printf("send->send\n");
		}

		//InterlockedAdd((LONG *)&session->GetIOCount(), -1);
		if (InterlockedAdd((LONG *)&session->GetIOCount(), -1) == 0)//&& !session->GetSocketActive())
		{
			//InterlockedAdd((LONG *)&out_check, 1);
			printf("delete session %d\n", session->GetID());

			AcquireSRWLockExclusive(&_this->sessionListLock);
			_this->sessionList.erase(session->GetID());
			session->Lock();
			ReleaseSRWLockExclusive(&_this->sessionListLock);

			_this->OnClientLeave(session->GetID());
			session->Unlock();
			delete session;
		}
	}


	return 0;
}

PROCRESULT CLanServer::CompleteRecvPacket(Session *session)
{
	LanServerHeader header;
	int recvQSize = session->GetRecvQ().GetUseSize();

	Packet payload;

	if (sizeof(header) > recvQSize)
		return NONE;

	session->GetRecvQ().Peek((char *)&header, sizeof(LanServerHeader));

	if (recvQSize < header.len + sizeof(header))
		return NONE;

	session->GetRecvQ().MoveReadPos(sizeof(LanServerHeader));

	if (session->GetRecvQ().Dequeue(payload, header.len) != header.len)
		return FAIL;

	//if (!PacketProc(session, 0, payload))
		//return FAIL;
	OnRecv(session->GetID(), &payload);

	return SUCCESS;
}

bool CLanServer::SendPacket(DWORD sessionID, Packet *p)
{
	AcquireSRWLockExclusive(&sessionListLock);
	auto iter = sessionList.find(sessionID);



	LanServerHeader header;
	if (iter == sessionList.end())
	{
		ReleaseSRWLockExclusive(&sessionListLock);
		return false;
	}
	Session *session = (*iter).second;
	session->Lock();
	ReleaseSRWLockExclusive(&sessionListLock);
	header.len = p->GetDataSize();

	if (session->GetSendQ().GetFreeSize() >= p->GetDataSize() + sizeof(header))
	{
		session->GetSendQ().Enqueue((char *)&header, sizeof(header));
		session->GetSendQ().Enqueue(*p);
	}
	else
	{
		session->Unlock();
		return false;
	}

	//session->SendPost();

	session->Unlock();
	return true;
}