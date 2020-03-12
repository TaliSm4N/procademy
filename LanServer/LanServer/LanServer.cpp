#include <iostream>
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "ws2_32")
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <process.h>
#include <cstring>
#include <map>
#include "Packet.h"
#include "RingBuffer.h"
#include "Session.h"
#include "LanServer.h"

CLanServer::CLanServer()
	:_sessionCount(0),_acceptTotal(0),_acceptTPS(0),_recvPacketTPS(0),_sendPacketTPS(0),_packetPoolAlloc(0),_packetPoolUse(0)
{
	
}

bool CLanServer::Start(int port,int workerCnt,bool nagle,int maxUser, bool monitoring)
{
	timeBeginPeriod(1);
	_port = port;
	_workerCnt = workerCnt;
	_nagle = nagle;
	_maxUser = maxUser;
	_monitoring = monitoring;

	//monitoring 초기화
	_sessionCount = 0;
	_acceptTotal = 0;
	_acceptTPS = 0;
	_recvPacketTPS = 0;
	_sendPacketTPS = 0;
	_packetPoolAlloc = 0;
	_packetPoolUse = 0;

	if (WSAStartup(MAKEWORD(2, 2), &_wsa) != 0) return false;

	_hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	if (_hcp == NULL) return false;

	InitializeSRWLock(&sessionListLock);
	
	


	_listenSock = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenSock == INVALID_SOCKET)
	{
		//printf("socket error\n");
		return -1;
	}

	ZeroMemory(&_sockAddr, sizeof(_sockAddr));
	_sockAddr.sin_family = AF_INET;
	_sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	_sockAddr.sin_port = htons(_port);
	int retval = bind(_listenSock, (SOCKADDR *)&_sockAddr, sizeof(_sockAddr));

	if (retval == SOCKET_ERROR)
	{
		//printf("bind error\n");
		return -1;
	}

	retval = listen(_listenSock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
	{
		//printf("listen error\n");
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

	wprintf(L"All thread closed\n");
	delete[] threadHandle;

	delete[] _hWokerThreads;
	delete[] _dwWOrkerThreadIDs;

	for (auto iter = sessionList.begin(); iter != sessionList.end();)
	{
		Session *session = iter->second;
		delete session;
	}
	sessionList.clear();
}

unsigned int WINAPI CLanServer::AcceptThread(LPVOID lpParam)
{

	//HANDLE hcp = (HANDLE)lpParam;

	CLanServer *_this = (CLanServer *)lpParam;

	Session *session;
	SOCKET sock;
	SOCKADDR_IN sockAddr;
	int addrLen;

	int retval;

	int id = 0;

	WCHAR IP[16];


	wprintf(L"Accept thread On\n");

	while (1)
	{
		addrLen = sizeof(sockAddr);
		sock = accept(_this->_listenSock, (SOCKADDR *)&(sockAddr), &addrLen);
		InterlockedIncrement((LONG *)&_this->_acceptTotal);
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
		
		InetNtopW(AF_INET, &sockAddr.sin_addr, IP, 16);
		if (!_this->OnConnectionRequest(IP, ntohs(sockAddr.sin_port)))
		{
			closesocket(sock);
			continue;
		}


		//printf("client connect %d\n", _this->GetSessionCount());

		session = new Session(sock, sockAddr, id++);

		//session->recvOn = false;
		//session->sendOn = false;

		InterlockedIncrement(&_this->_sessionCount);

		CreateIoCompletionPort((HANDLE)sock, _this->_hcp, (ULONG_PTR)session, 0);

		AcquireSRWLockExclusive(&_this->sessionListLock);
		_this->sessionList.insert(std::make_pair(session->GetID(), session));
		ReleaseSRWLockExclusive(&_this->sessionListLock);
		_this->OnClientJoin(session->GetID());

		InterlockedExchange8((CHAR *)&session->GetSocketActive(), TRUE);
		//accept 순간에 성공하지 않으면 session이 생성되지 않은거나 다름이 없음
		//if (!session->RecvPost())
		if (!_this->RecvPost(session))
		{ 
			_this->Disconnect(session->GetID());
			//AcquireSRWLockExclusive(&_this->sessionListLock);
			//_this->sessionList.erase(session->GetID());
			//InterlockedDecrement(&_this->_sessionCount);
			//session->Lock();
			//ReleaseSRWLockExclusive(&_this->sessionListLock);
			//
			//_this->OnClientLeave(session->GetID());
			//session->Unlock();
			//delete session;
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

	wprintf(L"Worker thread On\n");
	int test = 10;
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


		int ret = GetQueuedCompletionStatus(_this->_hcp, &transferred, (PULONG_PTR)&session, (LPOVERLAPPED *)&pOverlapped, INFINITE);
		//InterlockedIncrement((LONG *)&session->GetIOCount());

		
		if (transferred == 0 && session == 0 && pOverlapped == 0)
		{
			InterlockedDecrement((LONG *)&session->GetIOCount());
			//일반적으로 post를 통해 worker스레드를 종료시키는 목적으로 사용
			//printf("000\n");
			break;
		}

		if (ret == false && pOverlapped == NULL)
		{
			InterlockedIncrement((LONG *)&session->GetIOCount());

			//printf("ret error\n");
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
			//printf("transffer 0 %d\n", session->GetSocket());
		}

		if (pOverlapped->type == TYPE::RECV)
		{
			//InterlockedDecrement((LONG *)&session->GetIOCount());
			//InterlockedExchange8((char *)&session->recvOn, false);
			printf("recv Off\n");
			session->GetRecvQ().MoveWritePos(transferred);
			session->GetRecvQ().UnLock();
			while (1)
			{
				if (_this->CompleteRecvPacket(session) != SUCCESS)
					break;
				//printf("success\n");
			}
			printf("CRP end\n");
			//session->SendPost();
			////printf("recv->send\n");
			//printf("recv end %d\n", session->GetID());
			//session->RecvPost();
			_this->RecvPost(session);
		}
		else if (pOverlapped->type == TYPE::SEND)
		{
			//InterlockedExchange8((char *)&session->sendOn, false);
			printf("send off-----------------------------------\n");
			//SendFlag = false;
			//InterlockedDecrement((LONG *)&session->GetIOCount());

			//static int check=0;
			//static int before = 0;
			//InterlockedExchange((LONG *)&before, check);
			//if (InterlockedExchange((LONG *)&check, session->GetSendQ().GetReadPos() - session->GetSendQ().GetBufPtr())!=before)
			//{
			//	printf("---??\n");
			//}
			
			//wprintf(L"%10d ", session->GetSendQ().GetReadPos()- session->GetSendQ().GetBufPtr());
			session->GetSendQ().MoveReadPos(transferred);
			//wprintf(L"%10d\n", session->GetSendQ().GetReadPos() - session->GetSendQ().GetBufPtr());
			InterlockedExchange8(&session->GetSendFlag(), 1);

			_this->SendPost(session);
			printf("lunlun|||||||||||||||||||||||||||||||||||||||||||||\n");

			_this->OnSend(session->GetID(),transferred);
			//printf("send end %d\n",session->GetID());
		}

		//InterlockedAdd((LONG *)&session->GetIOCount(), -1);
		if (InterlockedDecrement((LONG *)&session->GetIOCount()) == 0)//&& !session->GetSocketActive())
		{
			_this->Disconnect(session->GetID());
		}
	}


	return 0;
}

bool CLanServer::Disconnect(DWORD sessionID)
{
	AcquireSRWLockExclusive(&sessionListLock);
	auto iter = sessionList.find(sessionID);
	if (iter == sessionList.end())
	{
		ReleaseSRWLockExclusive(&sessionListLock);
		return false;
	}
	Session *session = iter->second;
	sessionList.erase(sessionID);
	InterlockedDecrement(&_sessionCount);
	session->Lock();
	ReleaseSRWLockExclusive(&sessionListLock);

	OnClientLeave(session->GetID());
	session->Unlock();
	delete session;

	return true;
}

unsigned int WINAPI CLanServer::MonitorThread(LPVOID lpParam)
{
	CLanServer *_this = (CLanServer *)lpParam;
	int tick = timeGetTime();
	DWORD acceptBefore = 0;
	DWORD recvBefore = 0;
	DWORD sendBefore = 0;
	wprintf(L"Monitoring Thread On\n");
	while (1)
	{
		if (timeGetTime() - tick >= 1000)
		{
			tick += 1000;
			InterlockedExchange64(&_this->_acceptTPS, _this->_acceptTotal - acceptBefore);
			acceptBefore += _this->_acceptTPS;

			recvBefore = _this->_recvPacketCounter;
			InterlockedExchange64(&_this->_recvPacketCounter, 0);
			sendBefore = _this->_sendPacketCounter;
			InterlockedExchange64(&_this->_sendPacketCounter, 0);

			
			InterlockedExchange64((LONG64 *)&_this->_recvPacketTPS, recvBefore);
			InterlockedExchange64((LONG64 *)&_this->_sendPacketTPS, sendBefore);
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
	InterlockedIncrement64((LONG64 *)&_recvPacketCounter);
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
	SendPost(session);
	InterlockedIncrement64((LONG64 *)&_sendPacketCounter);
	session->Unlock();
	return true;
}

bool CLanServer::RecvPost(Session *session)
{
	if (!session->GetSocketActive())
		return false;
	//printf("---%d\n", sessionID);
	session->GetRecvQ().Lock();
	WSABUF wsabuf[2];
	wsabuf[0].len = session->GetRecvQ().DirectEnqueueSize();
	wsabuf[0].buf = session->GetRecvQ().GetWritePos();
	wsabuf[1].len = session->GetRecvQ().GetFreeSize() - session->GetRecvQ().DirectEnqueueSize();
	wsabuf[1].buf = session->GetRecvQ().GetBufPtr();


	DWORD flags = 0;

	InterlockedAdd((LONG *)&session->GetIOCount(), 1);

	//InterlockedExchange8((char *)&session->recvOn, true);
	printf("recv On\n");

	int retval = WSARecv(session->GetSocket(), wsabuf, 2, NULL, &flags, (OVERLAPPED *)&session->GetRecvOverlap(), NULL);
	//printf("recv\n");
	if (retval == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		//if (WSAGetLastError() != ERROR_IO_PENDING)
		if (err != ERROR_IO_PENDING)
		{
			//printf("%d\n",err);
			//printf("Not Overlapped Recv I/O %d\n", sessionID);
			if (InterlockedDecrement((LONG *)&session->GetIOCount()) == 0)//&& !session->GetSocketActive())
			{
				Disconnect(session->GetID());
			}
			//InterlockedExchange8((char *)&session->recvOn, false);
			printf("recv Off\n");
			//check delete

			return false;
		}
	}

	return true;
}
bool CLanServer::SendPost(Session *session)
{
	if (!session->GetSocketActive())
		return false;
	//sendQ.Lock();
	
	if (session->GetSendQ().GetUseSize() <= 0)
	{
		//volatile int test;
		//printf("use size\n");
		//test = 1;
		//sendQ.UnLock();
		return false;
	}

	if (InterlockedExchange8(&session->GetSendFlag(), 0) == 0)
	{
		//volatile int test;
		//printf("flag\n");
		//test = 1;
		return false;
	}
	//printf("%d---\n", sessionID);
	WSABUF wsabuf[2];

	wsabuf[0].len = session->GetSendQ().DirectDequeueSize();
	wsabuf[0].buf = session->GetSendQ().GetReadPos();
	wsabuf[1].len = session->GetSendQ().GetUseSize() - session->GetSendQ().DirectDequeueSize();
	wsabuf[1].buf = session->GetSendQ().GetBufPtr();


	DWORD flags = 0;
	InterlockedAdd((LONG *)&session->GetIOCount(), 1);

	//InterlockedExchange8((char *)&session->sendOn, true);
	printf("send On\n");
	int retval = WSASend(session->GetSocket(), wsabuf, 2, NULL, flags, (OVERLAPPED *)&session->GetSendOverlap(), NULL);
	//printf("send\n");
	if (retval == SOCKET_ERROR)
	{
		int err;
		if ((err = WSAGetLastError()) != ERROR_IO_PENDING)
		{
			//printf("Not Overlapped Send I/O %d, %d\n",session->GetID(),err);
			printf("send Off %d\n",err);
			InterlockedExchange8(&session->GetSendFlag(), 1);
			if (InterlockedDecrement((LONG *)&session->GetIOCount()) == 0)//&& !session->GetSocketActive())
			{
				Disconnect(session->GetID());
			}
			//InterlockedExchange8((char *)&session->sendOn, false);
			//check delete
			//sendQ.UnLock();
			return false;
		}
	}
	//sendQ.UnLock();
	return true;
}