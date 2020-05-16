#include "MMOLib.h"

CMMOServer::CMMOServer(int iMaxSession,bool monitoring)
	:_monitoring(monitoring),_maxSession(iMaxSession)
{
	_pSessionArray = new Session*[_maxSession];
}

CMMOServer::~CMMOServer()
{
}

bool CMMOServer::Start(WCHAR *szListenIP, int iPort, int iWorkerThread, bool bEnableNagle, BYTE byPacketCode, BYTE byPacketKey)
{
	WSADATA wsa;
	sockaddr_in sockAddr;

	//debug확인 후 없애기
	GET_AUTHCOUNT();
	GET_GAMECOUNT();
	GET_USERCOUNT();
	//debug확인 후 없애기


	//변수 초기화
	timeBeginPeriod(1);
	//memcpy(_szListenIP, szListenIP, sizeof(WCHAR) * 16);

	if(szListenIP!=NULL)
		wcscpy_s(_szListenIP, szListenIP);
	_iListenPort = iPort;
	_iWorkerThread = iWorkerThread;
	_bEnableNagle = bEnableNagle;
	_byPacketCode = byPacketCode;


	//monitoring변수 초기화

	_Monitor_AcceptSocket=0;
	_Monitor_AcceptFail=0;
	_Monitor_SessionAllMode=0;
	_Monitor_SessionAuthMode=0;
	_Monitor_SessionGameMode=0;
	_Monitor_Counter_AuthUpdate=0;
	_Monitor_Counter_GameUpdate=0;
	_Monitor_Counter_Accept=0;
	_Monitor_Counter_PacketProc=0;
	_Monitor_Counter_PacketSend=0;

	//packet 초기화
	Packet::Init(byPacketKey, byPacketCode);

	//세션리스트 설정
	//contents에서 하도록 유도하기
	
	for (int i = 0; i < _maxSession; i++)
	{
		_BlankSessionStack.Push(i);
	}


	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return false;

	_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	if (_hIOCP == NULL) return false;

	//InitializeSRWLock(&_usedSessionLock);



	_ListenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_ListenSocket == INVALID_SOCKET)
	{
		SYSLOG_LOG(L"Lib", LOG_ERROR, L"Listen Sock Error");
		return false;
	}

	int optval = 0;
	int retval = setsockopt(_ListenSocket, SOL_SOCKET, SO_RCVBUF, (char *)&optval, sizeof(optval));
	if (retval == SOCKET_ERROR)
	{
		SYSLOG_LOG(L"Lib", LOG_ERROR, L"SETSOCKET Error");
		int err = GetLastError();
		InterlockedIncrement(&_Monitor_AcceptFail);
		closesocket(_ListenSocket);
		return false;
		//return -1;
	}

	ZeroMemory(&sockAddr, sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	sockAddr.sin_port = htons(_iListenPort);
	retval = bind(_ListenSocket, (SOCKADDR *)&sockAddr, sizeof(sockAddr));

	if (retval == SOCKET_ERROR)
	{
		SYSLOG_LOG(L"Lib", LOG_ERROR, L"bind Error");
		return false;
	}

	retval = listen(_ListenSocket, SOMAXCONN);
	if (retval == SOCKET_ERROR)
	{
		SYSLOG_LOG(L"Lib", LOG_ERROR, L"listen Error");
		return false;
	}


	if (_bEnableNagle)
	{
		int optVal = true;
		setsockopt(_ListenSocket, IPPROTO_TCP, TCP_NODELAY, (char *)&optVal, sizeof(optVal));
	}

	if (_monitoring)
	{
		_hMonitorThread = (HANDLE)_beginthreadex(NULL, 0, MonitorThread, this, 0, NULL);
	}

	_hAcceptThread = (HANDLE)_beginthreadex(NULL, 0, AcceptThread, this, 0, NULL);


	for (int i = 0; i < _iWorkerThread; i++)
	{
		_hIOCPWorkerThread[i] = (HANDLE)_beginthreadex(NULL, 0, IOCPWorkerThread, this, 0, NULL);

		if (_hIOCPWorkerThread[i] == NULL) return false;
	}
	
	_hAuthThread = (HANDLE)_beginthreadex(NULL, 0, AuthThread, this, 0, NULL);
	_hGameUpdateThread = (HANDLE)_beginthreadex(NULL, 0, GameUpdateThread, this, 0, NULL);

	
	_hSendThread = (HANDLE)_beginthreadex(NULL, 0, SendThread, this, 0, NULL);
	

	return true;
}

bool CMMOServer::Stop(void)
{
	return true;
}

void CMMOServer::SetSessionArray(int iArrayIndex, Session *pSession)
{
	_pSessionArray[iArrayIndex] = pSession;
	pSession->SetIndex(iArrayIndex);
	//pSession = _pSessionArray[iArrayIndex];
}

unsigned __stdcall	CMMOServer::AcceptThread(void *pParam)
{
	CMMOServer *_this = (CMMOServer *)pParam;
	return _this ->AcceptThread_update();
}
bool				CMMOServer::AcceptThread_update(void)
{
	CLIENT_CONNECT_INFO *connectInfo;

	SOCKADDR_IN sockAddr;
	
	int addrLen;


	while (1)
	{
		addrLen = sizeof(sockAddr);

		connectInfo = _MemoryPool_ConnectInfo.Alloc();
		
		connectInfo->sock = accept(_ListenSocket, (SOCKADDR *)&(sockAddr), &addrLen);
		PRO_BEGIN(L"ACCEPT_PROC");
		
		
		if (connectInfo->sock == INVALID_SOCKET)
		{
			int err = WSAGetLastError();
			SYSLOG_LOG(L"Lib", LOG_ERROR, L"accept Error %d",err);
			InterlockedIncrement(&_Monitor_AcceptFail);
			continue;
		}
		InterlockedIncrement(&_Monitor_AcceptSocket);
		
		InetNtopW(AF_INET, &sockAddr.sin_addr, connectInfo->IP, 16);
		connectInfo->Port = sockAddr.sin_port;
		
		_AcceptSocketQueue.Enqueue(connectInfo);
		PRO_END(L"ACCEPT_PROC");
	}
}

unsigned __stdcall	CMMOServer::AuthThread(void *pParam)
{
	CMMOServer *_this = (CMMOServer *)pParam;
	return _this->AuthThread_update();
}
bool				CMMOServer::AuthThread_update(void)
{
	while (1)
	{
		PRO_BEGIN(L"Auth");
		ProcAuth_Accept();
		ProcAuth_Packet();
		OnAuth_Update();
		ProcAuth_StatusChange();

		InterlockedIncrement(&_Counter_AuthUpdate);
		PRO_END(L"Auth");
		Sleep(5);
	}

	return true;
}

void CMMOServer::ProcAuth_Accept()
{
	CLIENT_CONNECT_INFO *connectInfo;
	Session *session;
	int sessionPos;

	if (!_AcceptSocketQueue.Dequeue(&connectInfo))
		return;

	if (connectInfo == NULL)
	{
		SYSLOG_LOG(L"Lib", LOG_WARNING, L"ConnectInfo NULL");
		return;
	}
	PRO_BEGIN(L"Accept_Session");
	_BlankSessionStack.Pop(&sessionPos);
	session = _pSessionArray[sessionPos];

	InterlockedIncrement(&_Monitor_SessionAllMode);

	session->SetClientInfo(connectInfo);
	_MemoryPool_ConnectInfo.Free(connectInfo);

	session->SetMode(MODE_AUTH);
	InterlockedIncrement(&_Monitor_SessionAuthMode);

	session->OnAuth_ClientJoin();

	//recv걸기

	CreateIoCompletionPort((HANDLE)session->Socket(), _hIOCP, (ULONG_PTR)session, 0);

	InterlockedIncrement64(&session->IOCount());
	RecvPost(session);

	PRO_END(L"Accept_Session");

	if (InterlockedDecrement64(&session->IOCount()) == 0)
	{
		//session->Disconnect();
		session->Logout();
	}
	//
}
void CMMOServer::ProcAuth_Packet()
{
	Packet *p;
	Session *session;
	int count;
	for (int i = 0; i < _maxSession; i++)
	{
		session = _pSessionArray[i];
		if (session == NULL)
		{
			SYSLOG_LOG(L"Lib", LOG_WARNING, L"NULL Session Auth_Packet pos : %d", i);
			continue;
		}

		if (session->GetMode() != MODE_AUTH)
			continue;

		if (session->GetLogoutFlag())
		{
			session->SetMode(MODE_LOGOUT_IN_AUTH);
			continue;
		}

		//if (session->CompletePacketQ().empty())
		//	continue;
		//
		//p = session->CompletePacketQ().front();
		//if (p == NULL)
		//	continue;
		//session->CompletePacketQ().pop();

		//count = session->CompletePacketQ().GetUseCount();
		//
		//while (count-- > 0)
		//{
			if (!session->CompletePacketQ().Dequeue(&p))
				continue;


			PRO_BEGIN(L"AuthPacket");
			session->OnAuth_Packet(p);
			PRO_END(L"AuthPacket");
			Packet::Free(p);
		//}
	}
}


void CMMOServer::ProcAuth_StatusChange(void)
{
	Session *session;
	for (int i = 0; i < _maxSession; i++)
	{
		session = _pSessionArray[i];
		if (session == NULL)
		{
			SYSLOG_LOG(L"Lib", LOG_WARNING, L"NULL Session Auth_StatusChange pos : %d", i);
			continue;
		}

		if (session->GetMode() == MODE_LOGOUT_IN_AUTH)
		{
			InterlockedDecrement(&_Monitor_SessionAuthMode);
			session->SetMode(WAIT_LOGOUT);
		}
		else if (session->GetMode() == MODE_AUTH)
		{
			if (session->GetAuthToGameFlag())
			{
				InterlockedDecrement(&_Monitor_SessionAuthMode);
				session->SetMode(MODE_AUTH_TO_GAME);
			}
		}
		else
		{
			continue;
		}

		session->OnAuth_ClientLeave();
	}
}

unsigned __stdcall	CMMOServer::GameUpdateThread(void *pParam)
{
	CMMOServer *_this = (CMMOServer *)pParam;
	return _this->GameUpdateThread_update();
}

bool CMMOServer::GameUpdateThread_update(void)
{
	while (1)
	{
		PRO_BEGIN(L"Game");
		ProcGame_AuthToGame();
		ProcGame_Packet();
		OnAuth_Update();
		ProcGame_Logout();
		ProcGame_Release();

		InterlockedIncrement(&_Counter_GameUpdate);
		PRO_END(L"Game");
		Sleep(5);
	}

	return true;

	
}

void CMMOServer::ProcGame_AuthToGame()
{
	Session *session;
	for (int i = 0; i < _maxSession; i++)
	{
		session = _pSessionArray[i];
		if (session == NULL)
		{
			SYSLOG_LOG(L"Lib", LOG_WARNING, L"NULL Session Game_AuthToGame pos : %d",i);
			continue;
		}

		if (session->GetMode() != MODE_AUTH_TO_GAME)
			continue;

		session->SetMode(MODE_GAME);
		InterlockedIncrement(&_Monitor_SessionGameMode);

		session->OnGame_ClientJoin();
	}
}

void CMMOServer::ProcGame_Packet()
{
	Packet *p;
	Session *session;
	int count;
	for (int i = 0; i < _maxSession; i++)
	{
		session = _pSessionArray[i];
		if (session == NULL)
		{
			SYSLOG_LOG(L"Lib", LOG_WARNING, L"NULL Session Game_Packet pos : %d", i);
			continue;
		}

		if (session->GetMode() != MODE_GAME)
			continue;

		if (session->GetLogoutFlag())
		{
			session->SetMode(MODE_LOGOUT_IN_GAME);
			continue;
		}


		//if (session->CompletePacketQ().empty())
		//	continue;
		//
		//p = session->CompletePacketQ().front();
		//if (p == NULL)
		//	continue;
		//session->CompletePacketQ().pop();
		//count = session->CompletePacketQ().GetUseCount();
		//
		//while (count-- > 0)
		//{
			if (!session->CompletePacketQ().Dequeue(&p))
				continue;


			PRO_BEGIN(L"GamePacket");
			session->OnGame_Packet(p);
			PRO_END(L"GamePacket");
			Packet::Free(p);
		//}
	}
}

void CMMOServer::ProcGame_Logout()
{
	Session *session;
	for (int i = 0; i < _maxSession; i++)
	{
		session = _pSessionArray[i];
		if (session == NULL)
		{
			SYSLOG_LOG(L"Lib", LOG_WARNING, L"NULL Session Game_Logout pos : %d", i);
			continue;
		}

		if (session->GetMode() != MODE_LOGOUT_IN_GAME)
		{
			continue;
		}

		InterlockedDecrement(&_Monitor_SessionGameMode);

		session->SetMode(WAIT_LOGOUT);

		session->OnAuth_ClientLeave();
	}
}
void CMMOServer::ProcGame_Release()
{
	Session *session;
	for (int i = 0; i < _maxSession; i++)
	{
		session = _pSessionArray[i];
		if (session == NULL)
		{
			SYSLOG_LOG(L"Lib", LOG_WARNING, L"NULL Session Game_Release pos : %d", i);
			continue;
		}

		if (session->GetMode() != WAIT_LOGOUT)
			continue;
		PRO_BEGIN(L"Release");
		session->OnGame_ClientRelease();

		session->Reset();
		InterlockedDecrement(&_Monitor_SessionAllMode);

		if (session->GetMode() != MODE_NONE)
		{
			volatile int test = 1;
		}

		_BlankSessionStack.Push(session->GetIndex());
		PRO_END(L"Release");
	}
}

unsigned __stdcall	CMMOServer::IOCPWorkerThread(void *pParam)
{
	CMMOServer *_this = (CMMOServer *)pParam;
	return _this->IOCPWorkerThread_update();
}

bool				CMMOServer::IOCPWorkerThread_update(void)
{
	int ret;
	Session *session;
	OVERLAPPED *pOverlapped;
	DWORD transferred;
	while (1)
	{
		transferred = 0;
		session = NULL;
		pOverlapped = NULL;

		ret = GetQueuedCompletionStatus(_hIOCP, &transferred,(PULONG_PTR)&session, (LPOVERLAPPED *)&pOverlapped, INFINITE);

		if (transferred == 0 && session == 0 && pOverlapped == 0)
		{
			break;
		}

		if (ret == false && pOverlapped == NULL)
		{
			SYSLOG_LOG(L"Lib", LOG_ERROR, L"IOCP Error");
			CrashDump::Crash();
		}

		if (transferred == 0)
		{
			session->Disconnect();
		}

		//recv
		if (pOverlapped == &session->RecvOverlap())
		{
			session->RecvQ().MoveWritePos(transferred);

			PROCRESULT result;

			while (1)
			{
				PRO_BEGIN(L"COMPLETE_RECV");
				result = CompleteRecvPacket(session);
				PRO_END(L"COMPLETE_RECV");
				if (result != SUCCESS)
				{
					break;
				}
			}

			if (result != FAIL)
			{
				RecvPost(session);
			}
		}
		else if (pOverlapped == &session->SendOverlap())
		{
			for (int i = 0; i < session->GetSendCnt(); i++)
			{
				Packet *temp;
				session->SendQ().Dequeue(&temp);
				Packet::Free(temp);
			}
			session->SetSendCnt(0);

			InterlockedExchange(&session->SendFlag(), false);
		}

		if (InterlockedDecrement64(&session->IOCount()) == 0)
		{
			//session->Disconnect();
			session->Logout();
		}
	}

	return true;
}

PROCRESULT CMMOServer::CompleteRecvPacket(Session *session)
{
	int recvQSize = session->RecvQ().GetUseSize();

	Packet *payload;
	HEADER header;// = payload->GetHeaderPtr();

	if (sizeof(HEADER) > recvQSize)
	{
		//Packet::Free(payload);
		return NONE;
	}

	session->RecvQ().Peek((char *)&header, sizeof(HEADER));

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

	session->RecvQ().MoveReadPos(sizeof(HEADER));

	payload = Packet::Alloc();
	payload->RecvEncode();

	if (session->RecvQ().Dequeue(payload, header.len) != header.len)
	{

		Packet::Free(payload);
		return FAIL;
	}

	payload->PutHeader(&header);

	payload->decode();

	//payload 검증부분

	if (!payload->VerifyCheckSum())
	{
		Packet::Free(payload);
		return FAIL;
	}

	//session->CompletePacketQ().push(payload);
	session->CompletePacketQ().Enqueue(payload);
	InterlockedIncrement(&_Monitor_PacketProc);
	//Packet::Free(payload);

	return SUCCESS;
}

bool CMMOServer::RecvPost(Session *session, bool first)
{
	DWORD flags = 0;
	WSABUF wsabuf[2];

	wsabuf[0].len = session->RecvQ().DirectEnqueueSize();
	wsabuf[0].buf = session->RecvQ().GetWritePos();
	wsabuf[1].len = session->RecvQ().GetFreeSize() - session->RecvQ().DirectEnqueueSize();
	wsabuf[1].buf = session->RecvQ().GetBufPtr();

	InterlockedIncrement64(&session->IOCount());

	PRO_BEGIN(L"WSARecv");
	int retval = WSARecv(session->Socket(), wsabuf, 2, NULL, &flags, (OVERLAPPED *)&session->RecvOverlap(), NULL);
	

	if (retval == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != ERROR_IO_PENDING)
		{
			if (InterlockedDecrement64(&session->IOCount()) == 0)
			{
				//session->Disconnect();
				session->Logout();
			}
			PRO_END(L"WSARecv");
			return false;
		}
	}
	PRO_END(L"WSARecv");

	return true;
}

unsigned __stdcall	CMMOServer::SendThread(void *pParam)
{
	CMMOServer *_this = (CMMOServer *)pParam;
	return _this->SendThread_update();
}

bool				CMMOServer::SendThread_update(void)
{
	DWORD flags = 0;
	Session *session;
	en_SESSION_MODE mode;
	int peekCnt=0;// = session->GetSendQ()->GetUseCount();
	WSABUF wsabuf[1024];
	Packet *peekData[1024];

	while (1)
	{
		
		for (int i = 0; i < _maxSession; i++)
		{
			session = _pSessionArray[i];
			if (session == NULL)
			{
				SYSLOG_LOG(L"Lib", LOG_WARNING, L"NULL Session SendThread pos : %d", i);
				continue;
			}

			if (InterlockedExchange(&session->SendFlag(), true))
			{
				continue;
			}

			mode = session->GetMode();
			if (mode != MODE_AUTH && mode != MODE_GAME)
			{
				InterlockedExchange(&session->SendFlag(), false);
				continue;
			}
			peekCnt = session->SendQ().Peek(peekData, peekCnt);

			if (peekCnt == 0)
			{
				InterlockedExchange(&session->SendFlag(), false);
				continue;
			}

			for (int i = 0; i < peekCnt; i++)
			{
				wsabuf[i].buf = (char *)peekData[i]->GetSendPtr();
				wsabuf[i].len = peekData[i]->GetDataSize() + sizeof(HEADER);
				//memcpy(&last, peekData[i], sizeof(Packet));
				//last = *peekData[i];
			}

			session->SetSendCnt(peekCnt);

			InterlockedIncrement64(&session->IOCount());
			PRO_BEGIN(L"WSASend");
			int retval = WSASend(session->Socket(), wsabuf, peekCnt, NULL, flags, (OVERLAPPED *)&session->SendOverlap(), NULL);
			
			if (retval == SOCKET_ERROR)
			{
				int err;
				if ((err = WSAGetLastError()) != ERROR_IO_PENDING)
				{
					if (InterlockedDecrement64(&session->IOCount()) == 0)
					{
						PRO_END(L"WSASend");
						//session->Disconnect();
						session->Logout();
						continue;
					}
				}
			}
			PRO_END(L"WSASend");
			InterlockedAdd(&_Monitor_PacketSend, peekCnt);
		}

		Sleep(5);
	}
}

unsigned __stdcall	CMMOServer::MonitorThread(void *pParam)
{
	CMMOServer *_this = (CMMOServer *)pParam;
	return _this->MonitorThread_update();
}

bool				CMMOServer::MonitorThread_update(void)
{
	long beforeAccept=0;
	while (1)
	{
		_Monitor_Counter_Accept = _Monitor_AcceptSocket - beforeAccept;
		beforeAccept = _Monitor_AcceptSocket;

		_Monitor_Counter_PacketProc = _Monitor_PacketProc;
		_Monitor_PacketProc = 0;

		_Monitor_Counter_PacketSend = _Monitor_PacketSend;
		_Monitor_PacketSend = 0;

		_Monitor_Counter_AuthUpdate = _Counter_AuthUpdate;
		_Monitor_Counter_GameUpdate = _Counter_GameUpdate;

		_Counter_AuthUpdate = 0;
		_Counter_GameUpdate = 0;

		_Monitor_Counter_Packet = Packet::PacketUseCount();

		Sleep(1000);
	}

	return true;
}


//debug용 함수

long CMMOServer::GET_AUTHCOUNT()
{
	long cnt = 0;
	for (int i = 0; i < _maxSession; i++)
	{
		if (_pSessionArray[i]->GetMode() == MODE_AUTH)
			cnt++;
	}

	return cnt;
}
long CMMOServer::GET_GAMECOUNT()
{
	long cnt = 0;
	for (int i = 0; i < _maxSession; i++)
	{
		if (_pSessionArray[i]->GetMode() == MODE_GAME)
			cnt++;
	}

	return cnt;
}
long CMMOServer::GET_USERCOUNT()
{
	long cnt = 0;
	for (int i = 0; i < _maxSession; i++)
	{
		if (_pSessionArray[i]->GetMode() == MODE_NONE)
			continue;
		cnt++;
	}

	return cnt;
}