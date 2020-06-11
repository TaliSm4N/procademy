#include "MMOLib.h"

#define SIO_KEEPALIVE_VALS    _WSAIOW(IOC_VENDOR, 4)


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
	_Monitor_Counter_SendUpdate = 0;
	_Counter_SendUpdate = 0;
	_Monitor_Counter_Accept=0;
	_Monitor_Counter_PacketProc=0;
	_Monitor_Counter_PacketSend=0;

	_Monitor_SessionAuthToGameMode = 0;

	_sendThreadEvent = CreateWaitableTimer(NULL, FALSE, NULL);
	_gameThreadEvent = CreateWaitableTimer(NULL, FALSE, NULL);
	_authThreadEvent = CreateWaitableTimer(NULL, FALSE, NULL);
	_monitorThreadEvent = CreateWaitableTimer(NULL, FALSE, NULL);

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

	//tcp_keepalive keep;
	//
	//keep.onoff = 1;
	//keep.keepalivetime = 1000;
	//keep.keepaliveinterval = 10;
	//
	//WSAIoctl(_ListenSocket, SIO_KEEPALIVE_VALS, &keep, sizeof(tcp_keepalive), NULL, 0, NULL, NULL, NULL);

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

	optval = 0;
	retval = setsockopt(_ListenSocket, SOL_SOCKET, SO_SNDBUF, (char *)&optval, sizeof(optval));
	if (retval == SOCKET_ERROR)
	{
		SYSLOG_LOG(L"Lib", LOG_ERROR, L"SETSOCKET Error");
		int err = GetLastError();
		InterlockedIncrement(&_Monitor_AcceptFail);
		closesocket(_ListenSocket);
		return false;
		//return -1;
	}

	//optval = bEnableNagle;
	if(!bEnableNagle)
		retval = setsockopt(_ListenSocket, IPPROTO_TCP, TCP_NODELAY, (char *)&bEnableNagle, sizeof(bEnableNagle));
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

	
	for (int i = 0; i < 1; i++)
	{
		_hSendThread[i] = (HANDLE)_beginthreadex(NULL, 0, SendThread, this, 0, NULL);

		if (_hSendThread[i] == NULL) return false;
	}

	//_hSendThread = (HANDLE)_beginthreadex(NULL, 0, SendThread, this, 0, NULL);
	
	//_sendThreadEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	//_gameThreadEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	//_authThreadEvent = CreateEvent(NULL,FALSE,FALSE,NULL);

	

	LARGE_INTEGER liDueTime;
	liDueTime.QuadPart = -100000LL;

	SetWaitableTimer(_sendThreadEvent, &liDueTime, 20, NULL, NULL, FALSE);
	SetWaitableTimer(_gameThreadEvent, &liDueTime, 5, NULL, NULL, FALSE);
	SetWaitableTimer(_authThreadEvent, &liDueTime, 10, NULL, NULL, FALSE);
	SetWaitableTimer(_monitorThreadEvent, &liDueTime, 1000, NULL, NULL, FALSE);

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
		//PRO_BEGIN(L"ACCEPT_PROC");
		
		
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
		//PRO_END(L"ACCEPT_PROC");
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
		if (WaitForSingleObject(_authThreadEvent, INFINITE) != WAIT_OBJECT_0)
		{
			CrashDump::Crash();
		}

		//PRO_BEGIN(L"Auth");
		ProcAuth_Accept();
		ProcAuth_Packet();
		OnAuth_Update();
		ProcAuth_StatusChange();

		InterlockedIncrement(&_Counter_AuthUpdate);
		//PRO_END(L"Auth");
		
	}

	return true;
}

void CMMOServer::ProcAuth_Accept()
{
	CLIENT_CONNECT_INFO *connectInfo;
	Session *session;
	int sessionPos;
	while (_AcceptSocketQueue.GetUseCount()>0)
	{
		if (!_AcceptSocketQueue.Dequeue(&connectInfo))
		{
			break;
		}

		if (connectInfo == NULL)
		{
			SYSLOG_LOG(L"Lib", LOG_ERROR, L"ConnectInfo NULL");
			return;
		}
		_BlankSessionStack.Pop(&sessionPos);
		session = _pSessionArray[sessionPos];

		InterlockedIncrement(&_Monitor_SessionAllMode);

		session->SetClientInfo(connectInfo);
		_MemoryPool_ConnectInfo.Free(connectInfo);


		//recv걸기

		CreateIoCompletionPort((HANDLE)session->Socket(), _hIOCP, (ULONG_PTR)session, 0);

		session->OnAuth_ClientJoin();

		InterlockedIncrement64(&session->_IOCount);

		session->_Mode=MODE_AUTH;
		InterlockedIncrement(&_Monitor_SessionAuthMode);

		
		RecvPost(session);


		if (InterlockedDecrement64(&session->_IOCount) == 0)
		{
			//session->logAccept++;
			session->Logout();
			SYSLOG_LOG(L"Lib", LOG_WARNING, L"Logout accept : %d", session->GetIndex());
			//InterlockedIncrement(&_Monitor_Logout_Counter);
		}
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

		if (session->_Mode != MODE_AUTH)
			continue;

		if (session->_bLogoutFlag)
		{
			if (InterlockedExchange(&session->_lSendIO, true))
				continue;

			session->_Mode=MODE_LOGOUT_IN_AUTH;
			continue;
		}

		if (session->_CompletePacket.GetUseCount() <= 0)
			continue;


		count = AUTH_PROC_PACKET_MAX;
		while (count-- > 0&& session->_Mode==MODE_AUTH)
		{

			if (!session->_CompletePacket.Dequeue(&p))
				break;
			session->OnAuth_Packet(p);

			p->FreeTime = GetTickCount();

			InterlockedDecrement(&_Monitor_Counter_Recv);
			Packet::Free(p);
		}
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

		if (session->_Mode == MODE_LOGOUT_IN_AUTH)
		{
			//if (session->SendFlag())
			//if (InterlockedExchange(&session->SendFlag(), true))
			//	continue;

			InterlockedDecrement(&_Monitor_SessionAuthMode);
			session->_Mode=WAIT_LOGOUT;
			session->OnAuth_ClientLeave();
		}
		else if (session->_Mode == MODE_AUTH)
		{
			if (session->_bAuthToGameFlag)
			{
				InterlockedDecrement(&_Monitor_SessionAuthMode);
				session->_Mode=MODE_AUTH_TO_GAME;
				InterlockedIncrement(&_Monitor_SessionAuthToGameMode);
			}
		}

		
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
		if (WaitForSingleObject(_gameThreadEvent, INFINITE) != WAIT_OBJECT_0)
		{
			CrashDump::Crash();
		}

		//PRO_BEGIN(L"Game");
		ProcGame_AuthToGame();
		ProcGame_Packet();
		OnAuth_Update();
		ProcGame_Logout();
		ProcGame_Release();

		InterlockedIncrement(&_Counter_GameUpdate);
		//PRO_END(L"Game");
		//Sleep(5);
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

		if (session->_Mode == MODE_AUTH_TO_GAME)
		{
			InterlockedDecrement(&_Monitor_SessionAuthToGameMode);
			session->_Mode=MODE_GAME;
			InterlockedIncrement(&_Monitor_SessionGameMode);
			session->OnGame_ClientJoin();
		}

		if (session->_bLogoutFlag&&session->_Mode==MODE_GAME)
		{
			//session->SetLogoutFlag(false);

			if (InterlockedExchange(&session->_lSendIO, true))
				continue;
			
			session->_Mode=MODE_LOGOUT_IN_GAME;
			continue;
		}

		
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

		if (session->_Mode != MODE_GAME)
			continue;

		if (session->_CompletePacket.GetUseCount() <= 0)
			continue;

		//if (session->CompletePacketQ().empty())
		//	continue;
		//
		//p = session->CompletePacketQ().front();
		//if (p == NULL)
		//	continue;
		//session->CompletePacketQ().pop();
		//count = session->CompletePacketQ().GetUseCount();
		count = GAME_PROC_PACKET_MAX;

		while (count-- > 0 && session->_Mode==MODE_GAME)
		{

			if (!session->_CompletePacket.Dequeue(&p))
				break;
			session->OnGame_Packet(p);

			InterlockedDecrement(&_Monitor_Counter_Recv);
			Packet::Free(p);
		}
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

		if (session->_Mode != MODE_LOGOUT_IN_GAME)
		{
			continue;
		}

		//if (session->SendFlag())
		//if (InterlockedExchange(&session->SendFlag(), true))
		//	continue;

		InterlockedDecrement(&_Monitor_SessionGameMode);

		session->_Mode=WAIT_LOGOUT;

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

		if (session->_Mode != WAIT_LOGOUT)
			continue;
		//SYSLOG_LOG(L"Lib", LOG_WARNING, L"Session Game_Release pos : %d", session->GetIndex());
		session->OnGame_ClientRelease();

		InterlockedAdd(&_Monitor_Counter_Recv, -session->_CompletePacket.GetUseCount());

		//if (session->Socket() != INVALID_SOCKET)
		//{
		//	LOG(L"Lib", LOG_DEBUG, L"Not invalid socket %d sessionID %d", session->Socket(), session->iArrayIndex);
		//	//CrashDump::Crash();
		//}

		session->Reset();
		InterlockedIncrement(&_Monitor_Disconnect_Counter);

		InterlockedDecrement(&_Monitor_SessionAllMode);


		_BlankSessionStack.Push(session->iArrayIndex);
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
			//session->Logout();
			//InterlockedIncrement(&_Monitor_Transferred_Zero);

			if (pOverlapped == &session->_SendOverlapped)
			{
				//session->endRecvTime = GetTickCount();
				//SYSLOG_LOG(L"Lib", LOG_WARNING, L"disconnect send: %d", session->GetIndex());
				InterlockedExchange(&session->_lSendIO, false);
			}
			//else
			//{
			//	session->endSendTime = GetTickCount();
			//	SYSLOG_LOG(L"Lib", LOG_WARNING, L"disconnect recv: %d", session->GetIndex());
			//}

		}
		else if (pOverlapped == &session->_RecvOverlapped)
		{
			//session->endRecvTime = GetTickCount();
			//recv
			InterlockedDecrement(&_Monitor_RecvOverlap);

			session->_RecvQ.MoveWritePos(transferred);

			PROCRESULT result;

			while (1)
			{
				result = CompleteRecvPacket(session);
				if (result != SUCCESS)
				{
					break;
				}
			}

			if (result != FAIL&& !session->_bLogoutFlag)
			{
				RecvPost(session);
			}
		}
		else if (pOverlapped == &session->_SendOverlapped)
		{
			InterlockedDecrement(&_Monitor_SendOverlap);

			if (session->_iSendPacketSize != transferred)
			{
				SYSLOG_LOG(L"Lib", LOG_WARNING, L"not equal transferred %d : %d %d", session->iArrayIndex, session->_iSendPacketSize, transferred);
			}
	
			for (int i = 0; i < session->_iSendPacketCnt; i++)
			{
				Packet *temp;
				session->_SendQ.Dequeue(&temp);
				//temp->FreeTime = GetTickCount();
				//_Monitor_Send_Time += temp->FreeTime - temp->AllocTime;
				Packet::Free(temp);
			}
			session->_iSendPacketCnt=0;

			//if(!session->GetLogoutFlag())

			
			InterlockedExchange(&session->_lSendIO, false);
		}

		if (InterlockedDecrement64(&session->_IOCount) == 0)
		{
			//session->Disconnect();
			//session->logIOCP++;
			//if (pOverlapped == &session->_SendOverlapped)
			//{
			//	SYSLOG_LOG(L"Lib", LOG_WARNING, L"Logout iocp send: %d %d  - %d %d", session->GetIndex(), transferred, session->endRecvTime - session->recvTime, session->endSendTime - session->sendTime);
			//	//InterlockedExchange(&session->SendFlag(), false);
			//}
			//else
			//{
			//	SYSLOG_LOG(L"Lib", LOG_WARNING, L"Logout iocp recv: %d %d  - %d %d", session->GetIndex(), transferred, session->endRecvTime - session->recvTime, session->endSendTime - session->sendTime);
			//}
			
			session->Logout();
			//InterlockedIncrement(&_Monitor_Logout_Counter);
		}
	}

	return true;
}

PROCRESULT CMMOServer::CompleteRecvPacket(Session *session)
{
	int recvQSize = session->_RecvQ.GetUseSize();

	Packet *payload;
	HEADER header;// = payload->GetHeaderPtr();

	if (sizeof(HEADER) > recvQSize)
	{
		//Packet::Free(payload);
		return NONE;
	}

	
	session->_RecvQ.Peek((char *)&header, sizeof(HEADER));
	

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

	session->_RecvQ.MoveReadPos(sizeof(HEADER));

	payload = Packet::Alloc();
	payload->RecvEncode();

	if (session->_RecvQ.Dequeue(payload, header.len) != header.len)
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

	InterlockedIncrement(&_Monitor_Counter_Recv);

	session->_CompletePacket.Enqueue(payload);
	InterlockedIncrement(&_Monitor_PacketProc);
	//Packet::Free(payload);

	return SUCCESS;
}

bool CMMOServer::RecvPost(Session *session, bool first)
{
	RingBuffer *recvQ = &session->_RecvQ;
	DWORD flags = 0;
	WSABUF wsabuf[2];

	wsabuf[0].len = recvQ->DirectEnqueueSize();
	wsabuf[0].buf = recvQ->GetWritePos();
	wsabuf[1].len = recvQ->GetFreeSize() - recvQ->DirectEnqueueSize();
	wsabuf[1].buf = recvQ->GetBufPtr();

	InterlockedIncrement64(&session->_IOCount);

	
	ZeroMemory(&session->_RecvOverlapped, sizeof(OVERLAPPED));
	//PRO_BEGIN(L"WSARecv");
	//session->recvTime = GetTickCount();
	int retval = WSARecv(session->Socket(), wsabuf, 2, NULL, &flags, (OVERLAPPED *)&session->_RecvOverlapped, NULL);
	

	if (retval == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != ERROR_IO_PENDING)
		{
			//PRO_END(L"WSARecv");

			//session->Recverr = err;

			if (InterlockedDecrement64(&session->_IOCount) == 0)
			{
				//SYSLOG_LOG(L"Lib", LOG_WARNING, L"Logout recv : %d", session->GetIndex());
				//session->logRecv++;
				session->Logout();
				//InterlockedIncrement(&_Monitor_Logout_Counter);
			}
			
			return false;
		}
	}
	//PRO_END(L"WSARecv");
	InterlockedIncrement(&_Monitor_RecvOverlap);

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
	LockFreeQueue<Packet *> *sendQ;
	int peekCnt=0;// = session->GetSendQ()->GetUseCount();
	WSABUF wsabuf[1024];
	Packet *peekData[1024];

	while (1)
	{
		if (WaitForSingleObject(_sendThreadEvent, INFINITE) != WAIT_OBJECT_0)
		{
			CrashDump::Crash();
		}
		
		for (int i = 0; i < _maxSession; i++)
		{
			session = _pSessionArray[i];
			if (session == NULL)
			{
				SYSLOG_LOG(L"Lib", LOG_WARNING, L"NULL Session SendThread pos : %d", i);
				continue;
			}

			if (session->_bLogoutFlag)
				continue;

			mode = session->_Mode;
			if (mode != MODE_AUTH && mode != MODE_GAME)
			{
				continue;
			}

			sendQ = &session->_SendQ;

			if (InterlockedCompareExchange(&session->_lSendIO, true, false))
			{
				continue;
			}

			mode = session->_Mode;
			if (mode != MODE_AUTH && mode != MODE_GAME)
			{

				InterlockedCompareExchange(&session->_lSendIO, false, true);
				continue;
			}

			//if (session->SendQ().GetUseCount() <= 0)
			if(sendQ->GetUseCount() <= 0)
			{

				InterlockedCompareExchange(&session->_lSendIO, false, true);
				continue;
			}

			

			//if (session->SendFlag())
			//{
			//	continue;
			//}
			//InterlockedExchange(&session->SendFlag(), true);

			


			
			peekCnt = sendQ->Peek(peekData, peekCnt);

			if (peekCnt == 0)
			{

				InterlockedCompareExchange(&session->_lSendIO, false, true);
				continue;
			}

			session->_iSendPacketSize = 0;

			for (int i = 0; i < peekCnt; i++)
			{
				wsabuf[i].buf = (char *)peekData[i]->GetSendPtr();
				//wsabuf[i].len = 0;
				wsabuf[i].len = peekData[i]->GetDataSize() + sizeof(HEADER);
				session->_iSendPacketSize += wsabuf[i].len;
			}

			if (session->_iSendPacketSize == 0)
			{
				InterlockedCompareExchange(&session->_lSendIO, false, true);
				continue;
			}

			//session->SetSendCnt(peekCnt);
			session->_iSendPacketCnt = peekCnt;

			InterlockedAdd(&_Monitor_PacketSend, peekCnt);

			ZeroMemory(&session->_SendOverlapped, sizeof(OVERLAPPED));
			//PRO_BEGIN(L"WSASend");
			InterlockedIncrement64(&session->_IOCount);
			//session->sendTime = GetTickCount();
			int retval = WSASend(session->Socket(), wsabuf, peekCnt, NULL, flags, (OVERLAPPED *)&session->_SendOverlapped, NULL);

			
			if (retval == SOCKET_ERROR)
			{
				int err;
				if ((err = WSAGetLastError()) != ERROR_IO_PENDING)
				{
					//PRO_END(L"WSASend");

					//session->Senderr = err;

					

					InterlockedCompareExchange(&session->_lSendIO, false, true);

					if (InterlockedDecrement64(&session->_IOCount) == 0)
					{
						//SYSLOG_LOG(L"Lib", LOG_WARNING, L"Logout send : %d", session->GetIndex());
						//session->logSend++;
						session->Logout();
						//InterlockedIncrement(&_Monitor_Logout_Counter);
						
					}

					
					
					continue;
				}
			}
			//PRO_END(L"WSASend");

			InterlockedIncrement(&_Monitor_SendOverlap);
		}
		InterlockedIncrement(&_Counter_SendUpdate);
		//Sleep(5);
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
	long temp;
	while (1)
	{
		if (WaitForSingleObject(_monitorThreadEvent, INFINITE) != WAIT_OBJECT_0)
		{
			CrashDump::Crash();
		}

		_Monitor_Counter_Accept = _Monitor_AcceptSocket - beforeAccept;
		beforeAccept = _Monitor_AcceptSocket;

		
		_Monitor_Counter_PacketProc = _Monitor_PacketProc;
		InterlockedAdd(&_Monitor_PacketProc, -_Monitor_Counter_PacketProc);
		//_Monitor_PacketProc -= _Monitor_Counter_PacketProc;

		_Monitor_Counter_PacketSend = _Monitor_PacketSend;
		//_Monitor_PacketSend = 0;
		InterlockedAdd(&_Monitor_PacketSend, -_Monitor_Counter_PacketSend);

		_Monitor_Counter_AuthUpdate = _Counter_AuthUpdate;
		_Monitor_Counter_GameUpdate = _Counter_GameUpdate;
		_Monitor_Counter_SendUpdate = _Counter_SendUpdate;
		

		InterlockedAdd(&_Counter_AuthUpdate, -_Monitor_Counter_AuthUpdate);
		InterlockedAdd(&_Counter_GameUpdate, -_Monitor_Counter_GameUpdate);
		InterlockedAdd(&_Counter_SendUpdate, -_Monitor_Counter_SendUpdate);
		

		_Monitor_Counter_Packet = Packet::PacketUseCount();

		//Sleep(1000);
	}

	return true;
}
