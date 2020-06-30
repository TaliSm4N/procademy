


#include <iostream>
#include "NetServerLib.h"



CNetServer::CNetServer()
	:_sessionCount(0),_acceptTotal(0),_acceptTPS(0),_recvPacketTPS(0),_sendPacketTPS(0),_packetPoolAlloc(0),_packetPoolUse(0)
{
	
}

bool CNetServer::Config(const WCHAR *configFile, const WCHAR *block)
{
	TextParser parser;

	if (!parser.init(configFile))
		return false;

	if (!parser.SetCurBlock(block))
		return false;

	std::wstring str;

	if (!parser.SetCurBlock(block))
		return false;

	if (!parser.findItem(L"BIND_PORT", str))
		return false;
	_port = std::stoi(str);
	str.clear();


	if (parser.findItem(L"IOCP_WORKER_THREAD", str))
	{
		_workerCnt = std::stoi(str);
		str.clear();
	}
	else
	{
		_workerCnt = 5;
	}


	if (parser.findItem(L"IOCP_ACTIVE_THREAD", str))
	{
		_activeCnt = std::stoi(str);
		str.clear();
	}
	else
	{
		_activeCnt = _workerCnt / 2;
	}


	if (parser.findItem(L"CLIENT_MAX", str))
	{
		_maxUser = std::stoi(str);
		str.clear();
	}
	else
	{
		_maxUser = 10000;
	}

	//packet
	if (!parser.findItem(L"PACKET_CODE", str))
	{
		return false;
	}
	int code = std::stoi(str);
	str.clear();

	if (!parser.findItem(L"PACKET_KEY", str))
	{
		return false;
	}
	int key = std::stoi(str);
	str.clear();

	Packet::Init(key, code);

	if (parser.findItem(L"LOG_LEVEL", str))
	{

		if (wcscmp(str.c_str(), L"\"DEBUG\"") == 0)
		{
			SYSLOG_LEVEL(LOG_DEBUG);
		}
		else if (wcscmp(str.c_str(), L"\"WARNING\"") == 0)
		{
			SYSLOG_LEVEL(LOG_WARNING);
		}
		else if (wcscmp(str.c_str(), L"\"ERROR\"") == 0)
		{
			SYSLOG_LEVEL(LOG_ERROR);
		}
		str.clear();
	}

	_nagle = true;
	
	return true;
}
bool CNetServer::Start()
{
	int retval;
	timeBeginPeriod(1);
	_nagle = true;
	_monitoring = true;

	//monitoring 초기화
	_sessionCount = 0;
	_acceptTotal = 0;
	_acceptTPS = 0;
	_recvPacketTPS = 0;
	_sendPacketTPS = 0;
	_packetPoolAlloc = 0;
	_packetPoolUse = 0;
	_acceptFail = 0;


	//sessionList설정
	_sessionList = new Session[_maxUser];

	_sessionIndexStack = new LockFreeStack<int>();

	for (int i = _maxUser - 1; i >= 0; i--)
	{
		//_unUsedSessionStack.push(i);
		_sessionIndexStack->Push(i);
	}


	if (WSAStartup(MAKEWORD(2, 2), &_wsa) != 0) return false;

	_hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, _activeCnt);

	if (_hcp == NULL) return false;

	//InitializeSRWLock(&_usedSessionLock);



	_listenSock = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenSock == INVALID_SOCKET)
	{
		return -1;
	}

	int optval = 0;
	//retval = setsockopt(_listenSock, SOL_SOCKET, SO_RCVBUF, (char *)&optval, sizeof(optval));
	//if (retval == SOCKET_ERROR)
	//{
	//	int err = GetLastError();
	//	InterlockedIncrement((LONG *)&_acceptFail);
	//	closesocket(_listenSock);
	//	return -1;
	//	//return -1;
	//}

	retval = setsockopt(_listenSock, SOL_SOCKET, SO_SNDBUF, (char *)&optval, sizeof(optval));
	if (retval == SOCKET_ERROR)
	{
		int err = GetLastError();
		InterlockedIncrement((LONG *)&_acceptFail);
		closesocket(_listenSock);
		return -1;
		//return -1;
	}

	ZeroMemory(&_sockAddr, sizeof(_sockAddr));
	_sockAddr.sin_family = AF_INET;
	_sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	_sockAddr.sin_port = htons(_port);
	retval = bind(_listenSock, (SOCKADDR *)&_sockAddr, sizeof(_sockAddr));

	if (retval == SOCKET_ERROR)
	{
		return -1;
	}

	retval = listen(_listenSock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
	{
		return -1;
	}



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
		_hWokerThreads[i] = (HANDLE)_beginthreadex(NULL, 0, WorkerThread, this, 0, (unsigned int *)&_dwWOrkerThreadIDs[i]);

		if (_hWokerThreads[i] == NULL) return false;
	}
}

bool CNetServer::Start(int port,int workerCnt,bool nagle,int maxUser, bool monitoring)
{
	////////////////DEBUG

	////////////////DEBUG
	
	int retval;
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
	_acceptFail = 0;


	//sessionList설정
	_sessionList = new Session[_maxUser];

	_sessionIndexStack = new LockFreeStack<int>();

	for (int i = _maxUser - 1; i >= 0; i--)
	{
		//_unUsedSessionStack.push(i);
		_sessionIndexStack->Push(i);
	}
	Packet::Init();
	if (WSAStartup(MAKEWORD(2, 2), &_wsa) != 0) return false;

	_hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 3);

	if (_hcp == NULL) return false;

	//InitializeSRWLock(&_usedSessionLock);
	


	_listenSock = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenSock == INVALID_SOCKET)
	{
		return -1;
	}

	int optval = 0;
	retval = setsockopt(_listenSock, SOL_SOCKET, SO_SNDBUF, (char *)&optval, sizeof(optval));
	if (retval == SOCKET_ERROR)
	{
		int err = GetLastError();
		InterlockedIncrement((LONG *)&_acceptFail);
		closesocket(_listenSock);
		return -1;
		//return -1;
	}

	ZeroMemory(&_sockAddr, sizeof(_sockAddr));
	_sockAddr.sin_family = AF_INET;
	_sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	_sockAddr.sin_port = htons(_port);
	retval = bind(_listenSock, (SOCKADDR *)&_sockAddr, sizeof(_sockAddr));

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

bool CNetServer::ConfigStart(const WCHAR *configFile)
{
	TextParser parser;
	int retval;
	parser.init(configFile);

	parser.SetCurBlock(L"SERVER");

	std::wstring str;

	parser.findItem(L"BIND_PORT",str);
	int port = std::stoi(str);
	str.clear();

	parser.findItem(L"IOCP_WORKER_THREAD",str);
	int workerCnt = std::stoi(str);
	str.clear();

	parser.findItem(L"IOCP_ACTIVE_THREAD", str);
	int activeCnt = std::stoi(str);
	str.clear();
	
	parser.findItem(L"CLIENT_MAX", str);
	int maxUser = std::stoi(str);
	str.clear();


	//packet
	parser.findItem(L"PACKET_CODE", str);
	int code = std::stoi(str);
	str.clear();

	parser.findItem(L"PACKET_KEY", str);
	int key = std::stoi(str);
	str.clear();

	parser.findItem(L"LOG_LEVEL", str);
	std::wstring logLevel = str;
	str.clear();



	timeBeginPeriod(1);
	_port = port;
	_workerCnt = workerCnt;
	_nagle = true;
	_maxUser = maxUser;
	_monitoring = true;

	//monitoring 초기화
	_sessionCount = 0;
	_acceptTotal = 0;
	_acceptTPS = 0;
	_recvPacketTPS = 0;
	_sendPacketTPS = 0;
	_packetPoolAlloc = 0;
	_packetPoolUse = 0;
	_acceptFail = 0;


	Packet::Init(key,code);

	//sessionList설정
	_sessionList = new Session[_maxUser];

	_sessionIndexStack = new LockFreeStack<int>();

	for (int i = _maxUser - 1; i >= 0; i--)
	{
		_sessionIndexStack->Push(i);
	}


	if (WSAStartup(MAKEWORD(2, 2), &_wsa) != 0) return false;

	_hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, activeCnt);

	if (_hcp == NULL) return false;


	_listenSock = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenSock == INVALID_SOCKET)
	{
		return -1;
	}

	int optval = 0;
	//retval = setsockopt(_listenSock, SOL_SOCKET, SO_RCVBUF, (char *)&optval, sizeof(optval));
	//if (retval == SOCKET_ERROR)
	//{
	//	int err = GetLastError();
	//	InterlockedIncrement((LONG *)&_acceptFail);
	//	closesocket(_listenSock);
	//	return -1;
	//}

	retval = setsockopt(_listenSock, SOL_SOCKET, SO_SNDBUF, (char *)&optval, sizeof(optval));
	if (retval == SOCKET_ERROR)
	{
		int err = GetLastError();
		InterlockedIncrement((LONG *)&_acceptFail);
		closesocket(_listenSock);
		return -1;
		//return -1;
	}

	ZeroMemory(&_sockAddr, sizeof(_sockAddr));
	_sockAddr.sin_family = AF_INET;
	_sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	_sockAddr.sin_port = htons(_port);
	retval = bind(_listenSock, (SOCKADDR *)&_sockAddr, sizeof(_sockAddr));

	if (retval == SOCKET_ERROR)
	{
		return -1;
	}

	retval = listen(_listenSock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
	{
		return -1;
	}

	_nagle = true;

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
		_hWokerThreads[i] = (HANDLE)_beginthreadex(NULL, 0, WorkerThread, this, 0, (unsigned int *)&_dwWOrkerThreadIDs[i]);

		if (_hWokerThreads[i] == NULL) return false;
	}
}

void CNetServer::Stop()
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

	delete[] _sessionList;
	delete _sessionIndexStack;

}

unsigned int WINAPI CNetServer::AcceptThread(LPVOID lpParam)
{
	CNetServer *_this = (CNetServer *)lpParam;

	Session *session;
	SOCKET sock;
	SOCKADDR_IN sockAddr;
	int addrLen;

	int retval;
	DWORD idCount = 0;
	DWORD uniqueID=0;

	WCHAR IP[16];
	int sessionPos;



	while (1)
	{
		addrLen = sizeof(sockAddr);
		sock = accept(_this->_listenSock, (SOCKADDR *)&(sockAddr), &addrLen);
	
		if (sock == INVALID_SOCKET)
		{
			int err = WSAGetLastError();
			InterlockedIncrement((LONG *)&_this->_acceptFail);
			continue;
		}
		InterlockedIncrement((LONG *)&_this->_acceptTotal);
		
		InetNtopW(AF_INET, &sockAddr.sin_addr, IP, 16);
		if (!_this->OnConnectionRequest(IP, ntohs(sockAddr.sin_port)))
		{
			int err = GetLastError();
			InterlockedIncrement((LONG *)&_this->_connectionRequestFail);
			closesocket(sock);
			continue;
		}

		uniqueID = idCount;
		uniqueID <<= 16;
		if (!_this->_sessionIndexStack->Pop(&sessionPos))
		{
			CrashDump::Crash();
		}

		
		uniqueID += sessionPos;
		session = &_this->_sessionList[sessionPos];


		session->SetSessionInfo(sock, sockAddr, uniqueID);


		InterlockedIncrement(&_this->_sessionCount);

		CreateIoCompletionPort((HANDLE)sock, _this->_hcp, (ULONG_PTR)session, 0);

		InterlockedIncrement64(&session->GetIOCount());


		_this->OnClientJoin(session->GetID());
		
		
		//accept 순간에 성공하지 않으면 session이 생성되지 않은거나 다름이 없음
		

		_this->RecvPost(session);
		//recvPost이후부터 release가 가능하도록
		InterlockedExchange64(&session->GetReleaseFlag(), 0);

		
		if (InterlockedDecrement64(&session->GetIOCount()) == 0)
		{
			_this->ReleaseSession(session, uniqueID);
		}

		idCount++;
		uniqueID = -1;
		session = NULL;
		sock = INVALID_SOCKET;
		sessionPos = -1;
	}

	return 0;
}

unsigned int WINAPI CNetServer::WorkerThread(LPVOID lpParam)
{
	int retVal;
	CNetServer *_this = (CNetServer *)lpParam;

	Session *session;
	MyOverlapped *pOverlapped;
	DWORD transferred;

	DWORD id;
	
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


		if (transferred == 0 && session == 0 && pOverlapped == 0)
		{
			//InterlockedDecrement((LONG *)&session->GetIOCount());
			//일반적으로 post를 통해 worker스레드를 종료시키는 목적으로 사용
			break;
		}

		if (ret == false && pOverlapped == NULL)
		{
			CrashDump::Crash();
			//iocp 오류
			continue;
		}

		if (transferred == 0)
		{
			//단 zero바이트를 send해서 동기적인 작동을 유도하는 기법을 사용할 경우
			//이 부분의 코드가 항상 끊김처리인 것은 아님
			//그러나 보통은 사용하지 않는 기법
			//끊김처리


			session->Disconnect();
			if (pOverlapped == &session->GetSendOverlap())
			{
				InterlockedExchange8(&session->GetSendFlag(), 1);
			}

		}
		else if (pOverlapped == &session->GetRecvOverlap())
		{


			session->GetRecvQ().MoveWritePos(transferred);
			PROCRESULT result;
			while (1)
			{
				result = _this->CompleteRecvPacket(session);
				if (result != SUCCESS)
					break;
			}
			
			if (result != FAIL)
			{
				_this->RecvPost(session);
			}

		}
		else if (pOverlapped == &session->GetSendOverlap())
		{


			_this->OnSend(session->GetID(),transferred);


			for (int i = 0; i < session->GetSendPacketCnt(); i++)
			{
				Packet *temp;
				session->GetSendQ()->Dequeue(&temp);
				Packet::Free(temp);
			}
			session->SetSendPacketCnt(0);

			InterlockedExchange8(&session->GetSendFlag(), 1);

			//if(session->GetSendQ()->GetUseCount()>0)
				_this->SendPost(session);

		}

		id = session->GetID();

		if (InterlockedDecrement64(&session->GetIOCount()) == 0)
		{
			_this->ReleaseSession(session,id);
		}
		
	}


	return 0;
}

bool CNetServer::Disconnect(DWORD sessionID)
{
	Session *session = GetSession(sessionID);

	if (session == NULL)
	{
		return false;
	}
	
	session->Disconnect();

	PutSession(session);
	
	return true;
}

unsigned int WINAPI CNetServer::MonitorThread(LPVOID lpParam)
{
	CNetServer *_this = (CNetServer *)lpParam;
	//int tick = timeGetTime();
	LONG64 acceptBefore = 0;

	while (1)
	{
		//if (timeGetTime() - tick >= 1000)
		//{
		//	tick += 1000;
			InterlockedExchange64(&_this->_acceptTPS, _this->_acceptTotal - acceptBefore);
			acceptBefore += _this->_acceptTPS;

			InterlockedExchange64(&_this->_recvPacketTPS, _this->_recvPacketCounter);
			InterlockedExchange64(&_this->_recvPacketCounter, 0);

			InterlockedExchange64(&_this->_sendPacketTPS, _this->_sendPacketCounter);
			InterlockedExchange64(&_this->_sendPacketCounter, 0);
			InterlockedExchange64(&_this->_packetCount, Packet::PacketUseCount());
			Sleep(1000);
		//}
	}

	return 0;
}

//수정중
//비효율적임 수정해야함
PROCRESULT CNetServer::CompleteRecvPacket(Session *session)
{
	int recvQSize = session->GetRecvQ().GetUseSize();
	
	Packet *payload;
	NetServerHeader header;// = payload->GetHeaderPtr();

	if (sizeof(NetServerHeader) > recvQSize)
	{
		//Packet::Free(payload);
		return NONE;
	}

	session->GetRecvQ().Peek((char *)&header, sizeof(NetServerHeader));

	if (header.len > DEFAULT_PACKET_SIZE)
	{
		return FAIL;
	}

	if (recvQSize < header.len + sizeof(NetServerHeader))
	{
		//Packet::Free(payload);
		return NONE;
	}

	if (header.code != Packet::GetCode())
	{
		return FAIL;
	}

	session->GetRecvQ().MoveReadPos(sizeof(NetServerHeader));

	payload = Packet::Alloc();
	payload->RecvEncode();

	if (session->GetRecvQ().Dequeue(payload, header.len) != header.len)
	{

		Packet::Free(payload);
		return FAIL;
	}

	payload->PutHeader((char *)&header);

	payload->decode();

	//payload 검증부분
	
	if (!payload->VerifyCheckSum())
	{
		Packet::Free(payload);
		return FAIL;
	}

	OnRecv(session->GetID(), payload);
	Packet::Free(payload);
	InterlockedIncrement64((LONG64 *)&_recvPacketCounter);
	return SUCCESS;
}

bool CNetServer::SendPacket(DWORD sessionID, Packet *p)
{
	BYTE checkSum=0;

	Session *session = GetSession(sessionID);

	if (session == NULL)
	{
		return false;
	}


	InterlockedIncrement64((LONG64 *)&_sendPacketCounter);

	p->encode();

	if (session->GetSendQ()->GetFreeCount() > 0)
	{
		p->Ref();
		session->GetSendQ()->Enqueue(p);
	}
	SendPost(session);
	PutSession(session);
	return true;
}

bool CNetServer::RecvPost(Session *session)
{
	

	WSABUF wsabuf[2];
	wsabuf[0].len = session->GetRecvQ().DirectEnqueueSize();
	wsabuf[0].buf = session->GetRecvQ().GetWritePos();
	wsabuf[1].len = session->GetRecvQ().GetFreeSize() - session->GetRecvQ().DirectEnqueueSize();
	wsabuf[1].buf = session->GetRecvQ().GetBufPtr();


	DWORD flags = 0;

	InterlockedIncrement64(&session->GetIOCount());

	ZeroMemory(&session->GetRecvOverlap(), sizeof(MyOverlapped));
	session->GetRecvOverlap().type = RECV;


	//if (session->GetSocket() == INVALID_SOCKET)
	//{
	//
	//	if (InterlockedDecrement64(&session->GetIOCount()) == 0)
	//	{
	//		ReleaseSession(session, session->GetID());
	//	}
	//
	//	return false;
	//}

	int retval = WSARecv(session->GetSocket(), wsabuf, 2, NULL, &flags, (OVERLAPPED *)&session->GetRecvOverlap(), NULL);
	//int retval = WSARecv(INVALID_SOCKET, wsabuf, 2, NULL, &flags, (OVERLAPPED *)&session->GetRecvOverlap(), NULL);

	if (retval == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != ERROR_IO_PENDING)
		{
			DWORD id = session->GetID();

			if (InterlockedDecrement64(&session->GetIOCount()) == 0)
			{
				ReleaseSession(session,id);
			}

			return false;
		}
	}

	return true;
}
bool CNetServer::SendPost(Session *session)
{

	if (InterlockedExchange8(&session->GetSendFlag(), 0) == 0)
	{
		return false;
	}

	int peekCnt = session->GetSendQ()->GetUseCount();
	WSABUF wsabuf[1024];
	Packet *peekData[1024];

	peekCnt = session->GetSendQ()->Peek(peekData, peekCnt);

	if (peekCnt == 0)
	{
		InterlockedExchange8(&session->GetSendFlag(), 1);

		if (session->GetSendQ()->GetUseCount() > 0)
		{
			return SendPost(session);
		}

		return false;
	}

	for (int i = 0; i < peekCnt; i++)
	{
		wsabuf[i].buf = (char *)peekData[i]->GetSendPtr();
		wsabuf[i].len = peekData[i]->GetDataSize() + sizeof(NetServerHeader);
	}

	session->SetSendPacketCnt(peekCnt);

	DWORD flags = 0;
	InterlockedIncrement64(&session->GetIOCount());

	
	ZeroMemory(&session->GetSendOverlap(), sizeof(MyOverlapped));
	session->GetSendOverlap().type = SEND;


	int retval = WSASend(session->GetSocket(), wsabuf, peekCnt, NULL, flags, (OVERLAPPED *)&session->GetSendOverlap(), NULL);
	//int retval = WSASend(INVALID_SOCKET, wsabuf, peekCnt, NULL, flags, (OVERLAPPED *)&session->GetSendOverlap(), NULL);

	if (retval == SOCKET_ERROR)
	{
		int err;
		if ((err = WSAGetLastError()) != ERROR_IO_PENDING)
		{
			InterlockedExchange8(&session->GetSendFlag(), 1);
			DWORD id = session->GetID();

			if (InterlockedDecrement64(&session->GetIOCount()) == 0)
			{
				ReleaseSession(session,id);
			}

			return false;
		}
	}

	return true;
}

Session *CNetServer::GetSession(DWORD sessionID)
{
	DWORD pos = sessionID & 0xffff;

	if (pos == -1)
		return NULL;

	Session *session = &_sessionList[pos];

	if (session->GetID() != sessionID)
	{
		return NULL;
	}

	if (session->GetReleaseFlag())
		return NULL;


	//get session이 연속으로 2번 들어오면 iocount가 0이였던 session들도 이단계를 통과할 수 있는 위험이 있음
	if (InterlockedIncrement64(&session->GetIOCount()) == 1)
	{
		if (InterlockedDecrement64(&session->GetIOCount()) == 0)
		{
			ReleaseSession(session,sessionID);
			
		}

		return NULL;
	}

	if (session->GetID() != sessionID)
	{

		if (InterlockedDecrement64(&session->GetIOCount()) == 0)
		{
			ReleaseSession(session, sessionID);

		}

		return NULL;
	}


	//이미 release에 들어갔을 경우 대비
	//여기 들어가면 down client발생	
	if (session->GetReleaseFlag())
	{

		if (InterlockedDecrement64(&session->GetIOCount()) == 0)
		{
			ReleaseSession(session,sessionID);

		}
	
		return NULL;
	}

	


	return session;
}
void CNetServer::PutSession(Session *session)
{
	if (InterlockedDecrement64(&session->GetIOCount()) == 0)
	{
		ReleaseSession(session,session->GetID());
	}
}

void CNetServer::ReleaseSession(Session *session,DWORD sessionID)
{
	IOChecker checker;
	DWORD id;

	checker.IOCount = 0;
	checker.releaseFlag = false;



	if (!InterlockedCompareExchange128((LONG64 *)session->GetIOBlock(), (LONG64)true, (LONG64)0, (LONG64 *)&checker))
	{
		return;
	}


	

	closesocket(session->GetSocket());
	//session->Disconnect();

	//id = session->GetID();
	//session->GetID() = 0;

	OnClientLeave(sessionID);
	

	//남은 send Packet 제거
	while (session->GetSendQ()->GetUseCount() != 0)
	{
		Packet *temp;
		session->GetSendQ()->Dequeue(&temp);
		Packet::Free(temp);
	}
	
	_sessionIndexStack->Push(sessionID&0xffff);
	InterlockedDecrement(&_sessionCount);

	return;
}