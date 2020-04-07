#include <iostream>
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "ws2_32")
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <process.h>
#include <cstring>
#include <map>
#include <stack>
#include "MemoryPool.h"
#include "Packet.h"
#include "RingBuffer.h"
#include "Session.h"
#include "PacketPtr.h"
#include "LanServer.h"


CLanServer::CLanServer()
	:_sessionCount(0),_acceptTotal(0),_acceptTPS(0),_recvPacketTPS(0),_sendPacketTPS(0),_packetPoolAlloc(0),_packetPoolUse(0)
{
	packetPool = new MemoryPool<Packet>(10000, true);
}

bool CLanServer::Start(int port,int workerCnt,bool nagle,int maxUser, bool monitoring)
{
	////////////////DEBUG

	////////////////DEBUG
	

	timeBeginPeriod(1);
	_port = port;
	_workerCnt = workerCnt;
	_nagle = nagle;
	_maxUser = maxUser;
	_monitoring = monitoring;

	//monitoring �ʱ�ȭ
	_sessionCount = 0;
	_acceptTotal = 0;
	_acceptTPS = 0;
	_recvPacketTPS = 0;
	_sendPacketTPS = 0;
	_packetPoolAlloc = 0;
	_packetPoolUse = 0;
	_acceptFail = 0;


	//sessionList����
	_sessionList = new Session[_maxUser];

	for (int i = _maxUser - 1; i >= 0; i--)
	{
		_unUsedSessionStack.push(i);
	}


	if (WSAStartup(MAKEWORD(2, 2), &_wsa) != 0) return false;

	_hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	if (_hcp == NULL) return false;

	InitializeSRWLock(&_usedSessionLock);
	


	_listenSock = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenSock == INVALID_SOCKET)
	{
		return -1;
	}

	ZeroMemory(&_sockAddr, sizeof(_sockAddr));
	_sockAddr.sin_family = AF_INET;
	_sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	_sockAddr.sin_port = htons(_port);
	int retval = bind(_listenSock, (SOCKADDR *)&_sockAddr, sizeof(_sockAddr));

	if (retval == SOCKET_ERROR)
	{
		return -1;
	}

	retval = listen(_listenSock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
	{
		return -1;
	}

	_nagle = nagle;

	if (_nagle)
	{
		int optVal = true;
		setsockopt(_listenSock, IPPROTO_TCP, TCP_NODELAY, (char *)&optVal, sizeof(optVal));
	}

	if (_monitoring)
	{
		_hMonitorThread = (HANDLE)_beginthreadex(NULL, 0, MonitorThread, this, 0, (unsigned int *)&_dwMonitorThreadID);
	}

	_hAcceptThread = (HANDLE)_beginthreadex(NULL, 0, AcceptThread, this, 0, (unsigned int *)&_dwAcceptThreadID);

	_hWokerThreads = new HANDLE[_workerCnt];
	_dwWOrkerThreadIDs = new DWORD[_workerCnt];

	for (int i = 0; i < _workerCnt; i++)
	{
		_hWokerThreads[i]= (HANDLE)_beginthreadex(NULL, 0, WorkerThread, this, 0, (unsigned int *)&_dwWOrkerThreadIDs[i]);

		if (_hWokerThreads[i] == NULL) return false;
	}
}

void CLanServer::Stop()
{
	int handleCnt=_workerCnt+1;
	if (_monitoring)
		handleCnt++;
	HANDLE *threadHandle = new HANDLE[handleCnt];

	

	for (int i = 0; i <= _workerCnt; i++)
	{
		threadHandle[0] = _hWokerThreads[i];
	}

	threadHandle[_workerCnt] = _hAcceptThread;

	if (_monitoring)
	{
		threadHandle[handleCnt - 1] = _hMonitorThread;
	}

	WaitForMultipleObjects(handleCnt, threadHandle, TRUE, INFINITE);

	delete[] threadHandle;

	delete[] _hWokerThreads;
	delete[] _dwWOrkerThreadIDs;

}

unsigned int WINAPI CLanServer::AcceptThread(LPVOID lpParam)
{
	CLanServer *_this = (CLanServer *)lpParam;

	Session *session;
	SOCKET sock;
	SOCKADDR_IN sockAddr;
	int addrLen;

	int retval;
	DWORD idCount = 0;
	DWORD uniqueID=0;

	WCHAR IP[16];
	int sessionPos;


	wprintf(L"Accept thread On\n");

	while (1)
	{
		addrLen = sizeof(sockAddr);
		sock = accept(_this->_listenSock, (SOCKADDR *)&(sockAddr), &addrLen);
		InterlockedIncrement((LONG *)&_this->_acceptTotal);
		if (sock == INVALID_SOCKET)
		{
			InterlockedIncrement((LONG *)&_this->_acceptFail);
			continue;
		}

		int optval = 0;
		retval = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&optval, sizeof(optval));
		if (retval == SOCKET_ERROR)
		{
			InterlockedIncrement((LONG *)&_this->_acceptFail);
			closesocket(sock);
			return -1;
		}
		
		InetNtopW(AF_INET, &sockAddr.sin_addr, IP, 16);
		if (!_this->OnConnectionRequest(IP, ntohs(sockAddr.sin_port)))
		{
			InterlockedIncrement((LONG *)&_this->_connectionRequestFail);
			closesocket(sock);
			continue;
		}

		uniqueID = idCount;
		AcquireSRWLockExclusive(&_this->_usedSessionLock);
		sessionPos = _this->_unUsedSessionStack.top();
		_this->_unUsedSessionStack.pop();
		ReleaseSRWLockExclusive(&_this->_usedSessionLock);
		uniqueID <<= 16;
		uniqueID += sessionPos;
		session = &_this->_sessionList[sessionPos];
		session->SetSessionInfo(sock, sockAddr, uniqueID);


		InterlockedIncrement(&_this->_sessionCount);

		CreateIoCompletionPort((HANDLE)sock, _this->_hcp, (ULONG_PTR)session, 0);

		_this->OnClientJoin(session->GetID());

		InterlockedExchange8((CHAR *)&session->GetSocketActive(), TRUE);
		//accept ������ �������� ������ session�� �������� �����ų� �ٸ��� ����
		if (!_this->RecvPost(session))
		{ 
			_this->Disconnect(session->GetID());
		}
		else
		{
			//OnClientJoin�������� send�� ������ ������ ��� send����� ���ֱ� ���� �ڵ�
			_this->SendPost(session);
		}

		

		idCount++;
	}

	return 0;
}

unsigned int WINAPI CLanServer::WorkerThread(LPVOID lpParam)
{
	int retVal;
	CLanServer *_this = (CLanServer *)lpParam;

	Session *session;
	MyOverlapped *pOverlapped;
	DWORD transferred;

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

		if (transferred == 0 && session == 0 && pOverlapped == 0)
		{
			//InterlockedDecrement((LONG *)&session->GetIOCount());
			//�Ϲ������� post�� ���� worker�����带 �����Ű�� �������� ���
			break;
		}

		if (ret == false && pOverlapped == NULL)
		{
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
		}

		if (pOverlapped->type == TYPE::RECV)
		{
			session->GetRecvQ().MoveWritePos(transferred);
			while (1)
			{
				if (_this->CompleteRecvPacket(session) != SUCCESS)
					break;
			}
			_this->RecvPost(session);
		}
		else if (pOverlapped->type == TYPE::SEND)
		{
			//�ڵ�ȭ �׽�Ʈ
			//session->GetAutoSendQ().Lock();
			//for (int i = 0; i < session->GetSendPacketCnt(); i++)
			//{
			//	PacketPtr *temp;
			//	session->GetAutoSendQ().Dequeue((char *)&temp, sizeof(PacketPtr *));
			//	delete temp;
			//}
			//session->GetAutoSendQ().UnLock();
			//�ڵ�ȭ �׽�Ʈ
			session->GetSendQ().Lock();
			for (int i = 0; i < session->GetSendPacketCnt(); i++)
			{
				Packet *temp;
				session->GetSendQ().Dequeue((char *)&temp, sizeof(Packet *));
				//temp->Release();
				if (temp->UnRef())
				{
					_this->PacketFree(temp);
				}
			}
			session->GetSendQ().UnLock();
			InterlockedExchange8(&session->GetSendFlag(), 1);

			_this->SendPost(session);

			_this->OnSend(session->GetID(),transferred);
		}

		if (InterlockedDecrement((LONG *)&session->GetIOCount()) == 0)
		{
			_this->Disconnect(session->GetID());
		}
	}


	return 0;
}

bool CLanServer::Disconnect(DWORD sessionID)
{
	int idMask = 0xffff;
	sessionID &= idMask;
	Session *session = &_sessionList[sessionID];

	OnClientLeave(session->GetID());

	session->GetSendQ().Lock();

	//���� send Packet ����
	while(session->GetSendQ().GetUseSize()!=0)
	{
		Packet *temp;
		session->GetSendQ().Dequeue((char *)&temp, sizeof(Packet *));
		//temp->Release();
		if (temp->UnRef())
		{
			PacketFree(temp);
		}
	}

	session->GetSendQ().UnLock();

	InterlockedDecrement(&_sessionCount);

	AcquireSRWLockExclusive(&_usedSessionLock);
	_unUsedSessionStack.push(sessionID);

	
	ReleaseSRWLockExclusive(&_usedSessionLock);
	
	
	return true;
}

unsigned int WINAPI CLanServer::MonitorThread(LPVOID lpParam)
{
	CLanServer *_this = (CLanServer *)lpParam;
	int tick = timeGetTime();
	LONG64 acceptBefore = 0;

	while (1)
	{
		if (timeGetTime() - tick >= 1000)
		{
			tick += 1000;
			InterlockedExchange64(&_this->_acceptTPS, _this->_acceptTotal - acceptBefore);
			acceptBefore += _this->_acceptTPS;

			InterlockedExchange64(&_this->_recvPacketTPS, _this->_recvPacketCounter);
			InterlockedExchange64(&_this->_recvPacketCounter, 0);

			InterlockedExchange64(&_this->_sendPacketTPS, _this->_sendPacketCounter);
			InterlockedExchange64(&_this->_sendPacketCounter, 0);

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

	if (session->GetRecvQ().Dequeue(&payload, header.len) != header.len)
		return FAIL;

	OnRecv(session->GetID(), &payload);
	InterlockedIncrement64((LONG64 *)&_recvPacketCounter);
	return SUCCESS;
}

bool CLanServer::SendPacket(DWORD sessionID, Packet *p)
{
	LanServerHeader header;

	int idMask = 0xffff;
	sessionID &= idMask;
	Session *session = &_sessionList[sessionID];
	InterlockedIncrement64((LONG64 *)&_sendPacketCounter);

	header.len = p->GetDataSize();

	session->GetSendQ().Lock();
	if (session->GetSendQ().GetFreeSize() >= sizeof(p))
	{
		session->GetSendQ().Enqueue((char *)&p,sizeof(p));
	}
	session->GetSendQ().UnLock();
	SendPost(session);

	return true;
}

bool CLanServer::RecvPost(Session *session)
{

	if (!session->GetSocketActive())
	{
		return false;
	}

	WSABUF wsabuf[2];
	wsabuf[0].len = session->GetRecvQ().DirectEnqueueSize();
	wsabuf[0].buf = session->GetRecvQ().GetWritePos();
	wsabuf[1].len = session->GetRecvQ().GetFreeSize() - session->GetRecvQ().DirectEnqueueSize();
	wsabuf[1].buf = session->GetRecvQ().GetBufPtr();


	DWORD flags = 0;
	
	if (InterlockedIncrement((LONG *)&session->GetIOCount()) == 1)
	{
		volatile int test = 1;
	}

	int retval = WSARecv(session->GetSocket(), wsabuf, 2, NULL, &flags, (OVERLAPPED *)&session->GetRecvOverlap(), NULL);

	if (retval == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != ERROR_IO_PENDING)
		{
			if (InterlockedDecrement((LONG *)&session->GetIOCount()) == 0)
			{
				Disconnect(session->GetID());
			}

			return false;
		}
	}

	return true;
}
bool CLanServer::SendPost(Session *session)
{
	if (!session->GetSocketActive())
	{
		return false;
	}

	if (InterlockedExchange8(&session->GetSendFlag(), 0) == 0)
	{
		return false;
	}

	session->GetSendQ().Lock();
	if (session->GetSendQ().GetUseSize() <= 0)
	{
		InterlockedExchange8(&session->GetSendFlag(), 1);
		session->GetSendQ().UnLock();
		return false;
	}

	//�ڵ�ȭ �׽�Ʈ
	//session->GetAutoSendQ().Lock();
	//
	//if (session->GetAutoSendQ().GetUseSize() <= 0)
	//{
	//	InterlockedExchange8(&session->GetSendFlag(), 1);
	//	session->GetAutoSendQ().UnLock();
	//	return false;
	//}
	//
	//int autoSendQsize = session->GetAutoSendQ().GetUseSize();
	//int peekAutoCnt = autoSendQsize / sizeof(PacketPtr *);
	//PacketPtr *peekAutoData[1024];
	//WSABUF wsaAutobuf[1024];
	//
	//session->GetAutoSendQ().Peek((char *)peekAutoData, peekAutoCnt * sizeof(PacketPtr *));
	//
	//for (int i = 0; i < peekAutoCnt; i++)
	//{
	//	wsaAutobuf[i].buf = peekAutoData[i]->GetPacket()->GetBufferPtr();
	//	wsaAutobuf[i].len = peekAutoData[i]->GetPacket()->GetDataSize();
	//}
	//
	//session->SetSendPacketCnt(peekAutoCnt);
	//session->GetAutoSendQ().UnLock();
	//�ڵ�ȭ �׽�Ʈ
	
	int sendQsize = session->GetSendQ().GetUseSize();
	int peekCnt = sendQsize / sizeof(Packet *);
	WSABUF wsabuf[1024];
	Packet *peekData[1024];
	
	
	session->GetSendQ().Peek((char *)peekData, peekCnt * sizeof(Packet *));
	
	for (int i = 0; i < peekCnt; i++)
	{
		wsabuf[i].buf = peekData[i]->GetBufferPtr();
		wsabuf[i].len = peekData[i]->GetDataSize();
		peekData[i]->Ref();
	}
	session->SetSendPacketCnt(peekCnt);
	session->GetSendQ().UnLock();
	DWORD flags = 0;
	InterlockedIncrement((LONG *)&session->GetIOCount());

	
	int retval = WSASend(session->GetSocket(), wsabuf, peekCnt, NULL, flags, (OVERLAPPED *)&session->GetSendOverlap(), NULL);
	//int retval = WSASend(session->GetSocket(), wsaAutobuf, peekAutoCnt, NULL, flags, (OVERLAPPED *)&session->GetSendOverlap(), NULL);

	if (retval == SOCKET_ERROR)
	{
		int err;
		if ((err = WSAGetLastError()) != ERROR_IO_PENDING)
		{
			InterlockedExchange8(&session->GetSendFlag(), 1);
			if (InterlockedDecrement((LONG *)&session->GetIOCount()) == 0)
			{
				Disconnect(session->GetID());
			}
			return false;
		}
	}

	return true;
}

//bool CLanServer::AutoSendPacket(DWORD sessionID, PacketPtr *p)
//{
//	LanServerHeader header;
//
//	int idMask = 0xffff;
//	sessionID &= idMask;
//	Session *session = &_sessionList[sessionID];
//	InterlockedIncrement64((LONG64 *)&_sendPacketCounter);
//
//	header.len = p->GetPacket()->GetDataSize();
//
//	session->GetAutoSendQ().Lock();
//	if (session->GetAutoSendQ().GetFreeSize() >= sizeof(p))
//	{
//		session->GetAutoSendQ().Enqueue((char *)&p, sizeof(p));
//	}
//	session->GetAutoSendQ().UnLock();
//	SendPost(session);
//
//	return true;
//}