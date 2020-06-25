


#include <iostream>
#include "LanClientLib.h"

CLanClient::CLanClient()
	:_acceptTotal(0),_acceptTPS(0),_recvPacketTPS(0),_sendPacketTPS(0),_packetPoolAlloc(0),_packetPoolUse(0)
{
	//packetPool = new MemoryPoolTLS<Packet>(10000, true);
	Packet::Init();
}

bool CLanClient::initConnectInfo(const WCHAR *ip,int port)
{
	wcscpy_s(_ip, ip);
	_port = port;

	return true;
}

bool CLanClient::Config(const WCHAR *configFile, const WCHAR *block)
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
	_monitoring = true;

	return true;
}
bool CLanClient::Start()
{
	timeBeginPeriod(1);
	//_port = port;
	//monitoring 초기화
	_acceptTotal = 0;
	_acceptTPS = 0;
	_recvPacketTPS = 0;
	_sendPacketTPS = 0;
	_packetPoolAlloc = 0;
	_packetPoolUse = 0;
	_acceptFail = 0;

	if (WSAStartup(MAKEWORD(2, 2), &_wsa) != 0) return false;

	_hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, _activeCnt);

	if (_hcp == NULL) return false;

	if (_monitoring)
	{
		_hMonitorThread = (HANDLE)_beginthreadex(NULL, 0, MonitorThread, this, 0, (unsigned int *)&_dwMonitorThreadID);
	}


	_hWokerThreads = new HANDLE[_workerCnt];
	_dwWOrkerThreadIDs = new DWORD[_workerCnt];

	for (int i = 0; i < _workerCnt; i++)
	{
		_hWokerThreads[i] = (HANDLE)_beginthreadex(NULL, 0, WorkerThread, this, 0, (unsigned int *)&_dwWOrkerThreadIDs[i]);

		if (_hWokerThreads[i] == NULL) return false;
	}



	//InitializeSRWLock(&_usedSessionLock);



	return Connect();
}

bool CLanClient::Start(int workerCnt,bool nagle,bool monitoring)
{
	////////////////DEBUG

	////////////////DEBUG
	

	timeBeginPeriod(1);
	//_port = port;
	_workerCnt = workerCnt;
	_nagle = nagle;
	_monitoring = monitoring;

	//monitoring 초기화
	_acceptTotal = 0;
	_acceptTPS = 0;
	_recvPacketTPS = 0;
	_sendPacketTPS = 0;
	_packetPoolAlloc = 0;
	_packetPoolUse = 0;
	_acceptFail = 0;

	_nagle = nagle;

	if (WSAStartup(MAKEWORD(2, 2), &_wsa) != 0) return false;

	_hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	if (_hcp == NULL) return false;

	if (_monitoring)
	{
		_hMonitorThread = (HANDLE)_beginthreadex(NULL, 0, MonitorThread, this, 0, (unsigned int *)&_dwMonitorThreadID);
	}


	_hWokerThreads = new HANDLE[_workerCnt];
	_dwWOrkerThreadIDs = new DWORD[_workerCnt];

	for (int i = 0; i < _workerCnt; i++)
	{
		_hWokerThreads[i] = (HANDLE)_beginthreadex(NULL, 0, WorkerThread, this, 0, (unsigned int *)&_dwWOrkerThreadIDs[i]);

		if (_hWokerThreads[i] == NULL) return false;
	}

	

	//InitializeSRWLock(&_usedSessionLock);
	


	return Connect();
}

bool CLanClient::Connect()
{
	while (1)
	{
		sendFlag = 1;

		if (sendQ == NULL)
		{
			sendQ = new LockFreeQueue<Packet *>(1000);
		}

		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (_sock == INVALID_SOCKET)
		{
			continue;
			//return false;
		}




		_sockAddr.sin_port = htons(_port);



		ZeroMemory(&_sockAddr, sizeof(_sockAddr));
		_sockAddr.sin_family = AF_INET;
		InetPton(AF_INET, _ip, &_sockAddr.sin_addr);
		_sockAddr.sin_port = htons(_port);

		int optval = 0;
		int retval = setsockopt(_sock, SOL_SOCKET, SO_SNDBUF, (char *)&optval, sizeof(optval));
		if (retval == SOCKET_ERROR)
		{
			InterlockedIncrement((LONG *)&_acceptFail);
			closesocket(_sock);
			//return false;
			continue;
		}


		if (_nagle)
		{
			int optVal = true;
			setsockopt(_sock, IPPROTO_TCP, TCP_NODELAY, (char *)&optVal, sizeof(optVal));
		}

		retval = connect(_sock, (SOCKADDR *)&_sockAddr, sizeof(SOCKADDR_IN));

		if (retval == SOCKET_ERROR)
		{
			closesocket(_sock);
			//return false;
			continue;
		}

		CreateIoCompletionPort((HANDLE)_sock, _hcp, NULL, 0);
		InterlockedIncrement(&IOCount);
		OnServerJoin();

		RecvPost();

		InterlockedDecrement(&IOCount);

		return true;
	}
}

void CLanClient::Stop()
{
	int handleCnt=_workerCnt+1;
	if (_monitoring)
		handleCnt++;
	HANDLE *threadHandle = new HANDLE[handleCnt];

	

	for (int i = 0; i <= _workerCnt; i++)
	{
		threadHandle[0] = _hWokerThreads[i];
	}


	if (_monitoring)
	{
		threadHandle[handleCnt - 1] = _hMonitorThread;
	}

	WaitForMultipleObjects(handleCnt, threadHandle, TRUE, INFINITE);

	delete[] threadHandle;

	delete[] _hWokerThreads;
	delete[] _dwWOrkerThreadIDs;

}

unsigned int WINAPI CLanClient::WorkerThread(LPVOID lpParam)
{
	CLanClient *_this = (CLanClient *)lpParam;
	return _this->WorkerThread_update();
}

unsigned int WINAPI CLanClient::WorkerThread_update()
{
	///Session *session;
	OVERLAPPED *pOverlapped;
	DWORD transferred;
	ULONG_PTR key;

	while (1)
	{
		//이 두가지는 GetQueuedCompletionStatus가 실패하거나 하면 세팅되지 않음
		//이전 루프에서의 값을 보존하고 있음
		//이 경우 올바르지 않은 대상을 끊어낼 수 있는 문제가 생길 수 있음
		//일반적인 경우 초기화를 안해준다고 해서 문제가 생기진 않음
		//하지만 기능 변경 등의 문제로 문제가 발생할 위험 다소 있다
		transferred = 0;
		pOverlapped = NULL;


		int ret = GetQueuedCompletionStatus(_hcp, &transferred, &key, (LPOVERLAPPED *)&pOverlapped, INFINITE);

		if (transferred == 0 && pOverlapped == 0)
		{
			//InterlockedDecrement((LONG *)&session->GetIOCount());
			//일반적으로 post를 통해 worker스레드를 종료시키는 목적으로 사용
			break;
		}

		if (ret == false && pOverlapped == NULL)
		{
			//iocp 오류
			continue;
		}

		if (transferred == 0)
		{
			//단 zero바이트를 send해서 동기적인 작동을 유도하는 기법을 사용할 경우
			//이 부분의 코드가 항상 끊김처리인 것은 아님
			//그러나 보통은 사용하지 않는 기법
			//끊김처리
			Disconnect();
		}
		else if (pOverlapped == &recvOverlap)
		{

			recvQ.MoveWritePos(transferred);
			while (1)
			{
				if (CompleteRecvPacket() != SUCCESS)
					break;
			}
			RecvPost();
		}
		else if (pOverlapped == &sendOverlap)
		{
			if (sendFlag == 1)
			{
				volatile int test = 1;
			}

			for (int i = 0; i < sendPacketCnt; i++)
			{
				Packet *temp;
				sendQ->Dequeue(&temp);
				Packet::Free(temp);
			}

			if (sendFlag == 1)
			{
				volatile int test = 1;
			}

			InterlockedExchange8(&sendFlag, 1);

			SendPost();

			OnSend(transferred);
		}

		if (InterlockedDecrement(&IOCount) == 0)
		{
			Connect();
			//_this->ReleaseSession(session);
		}
	}


	return 0;
}

bool CLanClient::Disconnect()
{
	//Session *session = GetSession(sessionID);
	//
	//if (session == NULL)
	//{
	//	//ReleaseSession(session);
	//	return false;
	//}
	//
	//if (sessionID == session->GetID())
	//{
	//	InterlockedIncrement64(&_disconnectCount);
	//
	//
	//	closesocket(session->GetSocket());
	//	session->GetSocket() = INVALID_SOCKET;
	//	InterlockedExchange8((CHAR *)&session->GetSocketActive(), FALSE);
	//}
	//
	//PutSession(session);

	closesocket(_sock);

	return true;
}

unsigned int WINAPI CLanClient::MonitorThread_update()
{
	LONG64 acceptBefore = 0;

	while (1)
	{
		
		
			
		InterlockedExchange64(&_acceptTPS, _acceptTotal - acceptBefore);
		acceptBefore += _acceptTPS;

		InterlockedExchange64(&_recvPacketTPS,_recvPacketCounter);
		InterlockedExchange64(&_recvPacketCounter, 0);

		InterlockedExchange64(&_sendPacketTPS,_sendPacketCounter);
		InterlockedExchange64(&_sendPacketCounter, 0);
		InterlockedExchange64(&_packetCount, Packet::PacketUseCount());
		Sleep(1000);
		
	}

	return 0;
}

unsigned int WINAPI CLanClient::MonitorThread(LPVOID lpParam)
{
	CLanClient *_this = (CLanClient *)lpParam;
	return _this->MonitorThread_update();
}

//수정중
CLanClient::PROCRESULT CLanClient::CompleteRecvPacket()
{
	int recvQSize = recvQ.GetUseSize();

	Packet *payload = Packet::Alloc(LOCAL_TYPE);
	LanServerHeader header;// = payload->GetHeaderPtr();

	if (sizeof(LanServerHeader) > recvQSize)
	{
		Packet::Free(payload);
		return NONE;
	}

	recvQ.Peek((char *)&header, sizeof(LanServerHeader));

	if (recvQSize < header.len + sizeof(LanServerHeader))
	{
		Packet::Free(payload);
		return NONE;
	}

	recvQ.MoveReadPos(sizeof(LanServerHeader));

	if (recvQ.Dequeue(payload, header.len) != header.len)
	{
		Packet::Free(payload);
		return FAIL;
	}

	payload->PutHeader((char *)&header);

	OnRecv(payload);
	Packet::Free(payload);
	InterlockedIncrement64((LONG64 *)&_recvPacketCounter);
	return SUCCESS;
}

bool CLanClient::SendPacket(Packet *p)
{
	LanServerHeader header;

	header.len = p->GetDataSize();
	p->PutHeader((char *)&header);


	InterlockedIncrement64((LONG64 *)&_sendPacketCounter);

	header.len = p->GetDataSize();

	

	if (sendQ->GetFreeCount() > 0)
	{
		p->Ref();
		sendQ->Enqueue(p);
	}

	SendPost();
	
	return true;
}

bool CLanClient::RecvPost()
{

	WSABUF wsabuf[2];
	wsabuf[0].len = recvQ.DirectEnqueueSize();
	wsabuf[0].buf = recvQ.GetWritePos();
	wsabuf[1].len = recvQ.GetFreeSize() - recvQ.DirectEnqueueSize();
	wsabuf[1].buf = recvQ.GetBufPtr();


	DWORD flags = 0;

	InterlockedIncrement(&IOCount);

	int retval = WSARecv(_sock, wsabuf, 2, NULL, &flags, (OVERLAPPED *)&recvOverlap, NULL);

	if (retval == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != ERROR_IO_PENDING)
		{
			if (InterlockedDecrement(&IOCount) == 0)
			{
				Connect();
				//ReleaseSession(session);
			}

			return false;
		}
	}


	return true;
}
bool CLanClient::SendPost()
{


	if (InterlockedExchange8(&sendFlag, 0) == 0)
	{
		return false;
	}
	
	
	int peekCnt=0;
	WSABUF wsabuf[1024];
	Packet *peekData[1024];


	peekCnt = sendQ->Peek(peekData, peekCnt);

	if (peekCnt == 0)
	{
		InterlockedExchange8(&sendFlag, 1);

		if (sendQ->GetUseCount() > 0)
		{
			return SendPost();
		}

		return false;
	}

	for (int i = 0; i < peekCnt; i++)
	{
		wsabuf[i].buf = (char *)peekData[i]->GetSendPtr();
		wsabuf[i].len = peekData[i]->GetDataSize() + sizeof(LanServerHeader);
	}
	sendPacketCnt = peekCnt;
	//session->SetSendPacketCnt(peekCnt);
	DWORD flags = 0;
	InterlockedIncrement(&IOCount);

	
	int retval = WSASend(_sock, wsabuf, peekCnt, NULL, flags, (OVERLAPPED *)&sendOverlap, NULL);


	if (retval == SOCKET_ERROR)
	{
		int err;
		if ((err = WSAGetLastError()) != ERROR_IO_PENDING)
		{
			if (InterlockedDecrement(&IOCount) == 0)
			{
				Connect();
				//ReleaseSession(session);
			}
			return false;
		}
	}


	return true;
}




