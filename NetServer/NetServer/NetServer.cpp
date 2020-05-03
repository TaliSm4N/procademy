


#include <iostream>
#include "NetServerLib.h"



CNetServer::CNetServer()
	:_sessionCount(0),_acceptTotal(0),_acceptTPS(0),_recvPacketTPS(0),_sendPacketTPS(0),_packetPoolAlloc(0),_packetPoolUse(0)
{
	//packetPool = new MemoryPoolTLS<Packet>(10000, true);
	
}

bool CNetServer::Start(int port,int workerCnt,bool nagle,int maxUser, bool monitoring)
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

	_sessionIndexStack = new LockFreeStack<int>();

	for (int i = _maxUser - 1; i >= 0; i--)
	{
		//_unUsedSessionStack.push(i);
		_sessionIndexStack->Push(i);
	}

	Packet::Init();
	if (WSAStartup(MAKEWORD(2, 2), &_wsa) != 0) return false;

	_hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	if (_hcp == NULL) return false;

	//InitializeSRWLock(&_usedSessionLock);
	


	_listenSock = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenSock == INVALID_SOCKET)
	{
		return -1;
	}

	int optval = 0;
	int retval = setsockopt(_listenSock, SOL_SOCKET, SO_RCVBUF, (char *)&optval, sizeof(optval));
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

	//monitoring �ʱ�ȭ
	_sessionCount = 0;
	_acceptTotal = 0;
	_acceptTPS = 0;
	_recvPacketTPS = 0;
	_sendPacketTPS = 0;
	_packetPoolAlloc = 0;
	_packetPoolUse = 0;
	_acceptFail = 0;

	_disconnectCount = 0;
	_releaseCount = 0;
	_recvOverlap = 0;
	_sendOverlap = 0;
	_sessionGetCount = 0;
	_releaseClose = 0;

	Packet::Init(key,code);

	//sessionList����
	_sessionList = new Session[_maxUser];

	_sessionIndexStack = new LockFreeStack<int>();

	for (int i = _maxUser - 1; i >= 0; i--)
	{
		//_unUsedSessionStack.push(i);
		_sessionIndexStack->Push(i);
	}


	if (WSAStartup(MAKEWORD(2, 2), &_wsa) != 0) return false;

	_hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, activeCnt);

	if (_hcp == NULL) return false;

	//InitializeSRWLock(&_usedSessionLock);

	

	_listenSock = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenSock == INVALID_SOCKET)
	{
		return -1;
	}

	int optval = 0;
	int retval = setsockopt(_listenSock, SOL_SOCKET, SO_RCVBUF, (char *)&optval, sizeof(optval));
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


	wprintf(L"Accept thread On\n");

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

		//int optval = 0;
		//retval = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&optval, sizeof(optval));
		//if (retval == SOCKET_ERROR)
		//{
		//	int err = GetLastError();
		//	InterlockedIncrement((LONG *)&_this->_acceptFail);
		//	closesocket(sock);
		//	continue;
		//	//return -1;
		//}
		
		InetNtopW(AF_INET, &sockAddr.sin_addr, IP, 16);
		if (!_this->OnConnectionRequest(IP, ntohs(sockAddr.sin_port)))
		{
			int err = GetLastError();
			InterlockedIncrement((LONG *)&_this->_connectionRequestFail);
			closesocket(sock);
			continue;
		}

		uniqueID = idCount;
		//AcquireSRWLockExclusive(&_this->_usedSessionLock);
		_this->_sessionIndexStack->Pop(&sessionPos);
		//sessionPos = _this->_unUsedSessionStack.top();
		//_this->_unUsedSessionStack.pop();
		//ReleaseSRWLockExclusive(&_this->_usedSessionLock);
		uniqueID <<= 16;
		uniqueID += sessionPos;
		session = &_this->_sessionList[sessionPos];
		session->SetSessionInfo(sock, sockAddr, uniqueID);


		InterlockedIncrement(&_this->_sessionCount);

		CreateIoCompletionPort((HANDLE)sock, _this->_hcp, (ULONG_PTR)session, 0);

		//InterlockedIncrement64(&session->GetIOCount());
		
		InterlockedExchange8((CHAR *)&session->GetSocketActive(), TRUE);
		//InterlockedExchange64(&session->GetReleaseFlag(), false);
		_this->OnClientJoin(session->GetID());

		
		
		
		//accept ������ �������� ������ session�� �������� �����ų� �ٸ��� ����
		
		
		if (_this->RecvPost(session,true))
		{ 
			//_this->Disconnect(session->GetID());
			//OnClientJoin�������� send�� ������ ������ ��� send����� ���ֱ� ���� �ڵ�
			//_this->SendPost(session);
			//_this->SessionRelease(session);
		}

		//_this->PutSession(session);

		//recvPost���ĺ��� release�� �����ϵ���
		InterlockedExchange64(&session->GetReleaseFlag(), 0);
		if (InterlockedDecrement64(&session->GetIOCount()) == 0)
		{
			//����
			_this->ReleaseSession(session);
		}

		idCount++;
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

			
			//closesocket(session->GetSocket());
			_this->Disconnect(session->GetID());
		}

		if (pOverlapped->type == TYPE::RECV)
		{
			InterlockedDecrement64(&_this->_recvOverlap);

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

			//session->GetSendQ().Lock();
			//for (int i = 0; i < session->GetSendPacketCnt(); i++)
			//{
			//	Packet *temp;
			//	session->GetSendQ().Dequeue((char *)&temp, sizeof(Packet *));
			//	//temp->Release();
			//	//if (temp->UnRef())
			//	//{
			//		//_this->PacketFree(temp);
			//	Packet::Free(temp);
			//	//}
			//}
			//session->GetSendQ().UnLock();

			InterlockedDecrement64(&_this->_sendOverlap);

			for (int i = 0; i < session->GetSendPacketCnt(); i++)
			{
				Packet *temp;
				session->GetSendQ()->Dequeue(&temp);
				Packet::Free(temp);
			}
			session->SetSendPacketCnt(0);

			InterlockedExchange8(&session->GetSendFlag(), 1);

			_this->SendPost(session);

			_this->OnSend(session->GetID(),transferred);
		}

		if (InterlockedDecrement64(&session->GetIOCount()) == 0)
		{
			//_this->Disconnect(session->GetID());
			_this->ReleaseSession(session);
		}
	}


	return 0;
}

bool CNetServer::Disconnect(DWORD sessionID)
{
	//int idMask = 0xffff;
	//sessionID &= idMask;
	//Session *session = &_sessionList[sessionID];
	Session *session = GetSession(sessionID);

	if (session == NULL)
	{
		//ReleaseSession(session);
		return false;
	}


	if (sessionID == session->GetID())
	{
		if (InterlockedExchange8((CHAR *)&session->GetSocketActive(), FALSE))
		{
			InterlockedIncrement64(&_disconnectCount);
			//closesocket(session->GetSocket());
			//closesocket(socket);
			//shutdown(socket,SD_BOTH);

			SOCKET sock = session->GetSocket();
			if (InterlockedExchange(&session->GetSocket(), INVALID_SOCKET) != INVALID_SOCKET)
				closesocket(sock);
		}
		//session->GetSocket() = INVALID_SOCKET;
		
	}

	PutSession(session);
	
	return true;
}

unsigned int WINAPI CNetServer::MonitorThread(LPVOID lpParam)
{
	CNetServer *_this = (CNetServer *)lpParam;
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
			InterlockedExchange64(&_this->_packetCount, Packet::PacketUseCount());
		}
	}

	return 0;
}

//������
//��ȿ������ �����ؾ���
PROCRESULT CNetServer::CompleteRecvPacket(Session *session)
{
	int recvQSize = session->GetRecvQ().GetUseSize();
	
	Packet *payload;
	HEADER header;// = payload->GetHeaderPtr();

	if (sizeof(HEADER) > recvQSize)
	{
		//Packet::Free(payload);
		return NONE;
	}

	session->GetRecvQ().Peek((char *)&header, sizeof(HEADER));

	if (header.len > DEFAULT_PACKET_SIZE)
	{
		return FAIL;
	}

	if (recvQSize < header.len + sizeof(HEADER))
	{
		//Packet::Free(payload);
		return NONE;
	}

	if (header.code != Packet::GetCode())
	{
		return FAIL;
	}

	session->GetRecvQ().MoveReadPos(sizeof(HEADER));

	payload = Packet::Alloc();
	payload->RecvEncode();

	if (session->GetRecvQ().Dequeue(payload, header.len) != header.len)
	{

		Packet::Free(payload);
		return FAIL;
	}

	payload->PutHeader(&header);

	payload->decode();

	//payload �����κ�
	
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
	
	//int idMask = 0xffff;
	//sessionID &= idMask;
	//Session *session = &_sessionList[sessionID];
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
	else
	{
		volatile int test = 1;
	}

	//session->GetSendQ().Lock();
	//if (session->GetSendQ().GetFreeSize() >= sizeof(p))
	//{
	//	session->GetSendQ().Enqueue((char *)&p,sizeof(p));
	//}
	//session->GetSendQ().UnLock();
	SendPost(session);
	PutSession(session);
	return true;
}

bool CNetServer::RecvPost(Session *session,bool first)
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
	
	//if(!first)
	InterlockedIncrement64(&session->GetIOCount());
	
	//else
	//	session->acceptCheck = true;

	int retval = WSARecv(session->GetSocket(), wsabuf, 2, NULL, &flags, (OVERLAPPED *)&session->GetRecvOverlap(), NULL);
	

	if (retval == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != ERROR_IO_PENDING)
		{
			if (InterlockedDecrement64(&session->GetIOCount()) == 0)
			{
				Disconnect(session->GetID());
				//SessionRelease(session);
			}

			return false;
		}
	}

	InterlockedIncrement64(&_recvOverlap);

	return true;
}
bool CNetServer::SendPost(Session *session)
{
	if (!session->GetSocketActive())
	{
		return false;
	}

	//if (session->GetSendQ()->GetUseCount() <= 0)
	//{
	//	return false;
	//}

	if (InterlockedExchange8(&session->GetSendFlag(), 0) == 0)
	{
		return false;
	}

	//session->GetSendQ().Lock();
	//if (session->GetSendQ().GetUseSize() <= 0)
	//{
	//	InterlockedExchange8(&session->GetSendFlag(), 1);
	//	session->GetSendQ().UnLock();
	//	return false;
	//}

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
	
	
	//int sendQsize = session->GetSendQ().GetUseSize();
	//int peekCnt = sendQsize / sizeof(Packet *);
	int peekCnt = session->GetSendQ()->GetUseCount();
	WSABUF wsabuf[1024];
	Packet *peekData[1024];
	
	
	//session->GetSendQ().Peek((char *)peekData, peekCnt * sizeof(Packet *));
	
	//for (int i = 0; i < peekCnt; i++)
	//{
	//	//wsabuf[i].buf = peekData[i]->GetBufferPtr();
	//	wsabuf[i].buf = (char *)peekData[i]->GetSendPtr();
	//	wsabuf[i].len = peekData[i]->GetDataSize()+sizeof(HEADER);
	//	//peekData[i]->Ref();
	//}

	//if (peekCnt == 0)
	//{
	//	InterlockedExchange8(&session->GetSendFlag(), 1);
	//	return false;
	//}

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
		wsabuf[i].len = peekData[i]->GetDataSize() + sizeof(HEADER);
		//memcpy(&last, peekData[i], sizeof(Packet));
		//last = *peekData[i];
	}

	session->SetSendPacketCnt(peekCnt);
	//session->GetSendQ().UnLock();
	DWORD flags = 0;
	InterlockedIncrement64(&session->GetIOCount());

	
	int retval = WSASend(session->GetSocket(), wsabuf, peekCnt, NULL, flags, (OVERLAPPED *)&session->GetSendOverlap(), NULL);
	//int retval = WSASend(session->GetSocket(), wsaAutobuf, peekAutoCnt, NULL, flags, (OVERLAPPED *)&session->GetSendOverlap(), NULL);

	if (retval == SOCKET_ERROR)
	{
		int err;
		if ((err = WSAGetLastError()) != ERROR_IO_PENDING)
		{
			//InterlockedExchange8(&session->GetSendFlag(), 1);
			if (InterlockedDecrement64(&session->GetIOCount()) == 0)
			{
				//Disconnect(session->GetID());
				//SessionRelease(session);
				ReleaseSession(session);
			}
			//wprintf(L"%d-----------\n", err);
			return false;
		}
	}

	InterlockedIncrement64(&_sendOverlap);

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


	
	if (InterlockedIncrement64(&session->GetIOCount()) == 1)
	{
		if (InterlockedDecrement64(&session->GetIOCount()) == 0)
		{
			//����
			//Disconnect(sessionID);
			ReleaseSession(session);
			
		}
		return NULL;
	}

	//������ ���̴� ��Ʈ

	//���� ���� �Ŀ� down Client�� �߻��ϴ� ������ ����
	//��Ȯ������ ����
	if (session->GetID() != sessionID)
	{
		if (InterlockedDecrement64(&session->GetIOCount()) == 0)
		{
			//����
			//Disconnect(sessionID);
			ReleaseSession(session);
			//CrashDump::Crash();
			
		}
		return NULL;
	}

	//�̹� release�� ���� ��� ���
	//���� ���� down client�߻�	
	if (session->GetReleaseFlag())
	{
		if (InterlockedDecrement64(&session->GetIOCount()) == 0)
		{
			//����
			//Disconnect(sessionID);
			ReleaseSession(session);
			//CrashDump::Crash();
		}
	
		return NULL;
	}

	InterlockedIncrement64(&_sessionGetCount);

	return session;
}
void CNetServer::PutSession(Session *session)
{
	if (InterlockedDecrement64(&session->GetIOCount()) == 0)
	{
		//����
		ReleaseSession(session);
	}

	InterlockedDecrement64(&_sessionGetCount);
}

void CNetServer::ReleaseSession(Session *session)
{
	IOChecker checker;
	DWORD id;

	checker.IOCount = 0;
	checker.releaseFlag = false;

	
	//release�� ���� �ʾƵ� �� ��� Ż��
	//IOCount�� 0�� �ƴϰų� release�� ���� ���� ��
	if (!InterlockedCompareExchange128((LONG64 *)session->GetIOBlock(), (LONG64)true, (LONG64)1, (LONG64 *)&checker))
	{
		return;
	}
	
	//InterlockedIncrement64(&session->GetIOCount());
	id = session->GetID();
	session->GetID() = 0;
	OnClientLeave(id);

	if (InterlockedExchange8((CHAR *)&session->GetSocketActive(), FALSE))
	{
		SOCKET sock= session->GetSocket();
		if(InterlockedExchange(&session->GetSocket(), INVALID_SOCKET)!= INVALID_SOCKET)
			closesocket(sock);
		InterlockedIncrement64(&_releaseClose);
	}

	

	//���� send Packet ����
	while (session->GetSendQ()->GetUseCount() != 0)
	{
		Packet *temp;
		session->GetSendQ()->Dequeue(&temp);
		//temp->Release();
		//if (temp->UnRef())
		//{
			//PacketFree(temp);
		Packet::Free(temp);
		//}
	}

	//session->GetSendQ().Lock();
	//
	////���� send Packet ����
	//while(session->GetSendQ().GetUseSize()!=0)
	//{
	//	Packet *temp;
	//	session->GetSendQ().Dequeue((char *)&temp, sizeof(Packet *));
	//	//temp->Release();
	//	//if (temp->UnRef())
	//	//{
	//		//PacketFree(temp);
	//	Packet::Free(temp);
	//	//}
	//}
	//
	//session->GetSendQ().UnLock();

	//AcquireSRWLockExclusive(&_usedSessionLock);
	//_unUsedSessionStack.push(sessionID);

	
	_sessionIndexStack->Push(id&0xffff);
	InterlockedDecrement(&_sessionCount);
	//ReleaseSRWLockExclusive(&_usedSessionLock);

	
	InterlockedIncrement64(&_releaseCount);
	return;
}


//bool CNetServer::AutoSendPacket(DWORD sessionID, PacketPtr *p)
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
