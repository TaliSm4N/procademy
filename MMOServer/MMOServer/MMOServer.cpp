#include "MMOLib.h"

CMMOServer::CMMOServer(int iMaxSession,bool monitoring)
	:_monitoring(monitoring),_maxSession(iMaxSession)
{
}

bool CMMOServer::Start(WCHAR *szListenIP, int iPort, int iWorkerThread, bool bEnableNagle, BYTE byPacketCode, BYTE byPacketKey)
{
	WSADATA wsa;
	sockaddr_in sockAddr;

	//변수 초기화
	timeBeginPeriod(1);
	wcscpy(_szListenIP, szListenIP);
	_iListenPort = iPort;
	_iWorkerThread = iWorkerThread;
	_bEnableNagle = bEnableNagle;
	_byPacketCode = byPacketCode;


	//monitoring변수 초기화



	//packet 초기화
	Packet::Init(byPacketKey, byPacketCode);

	//세션리스트 설정
	//contents에서 하도록 유도하기
	//


	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return false;

	_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	if (_hIOCP == NULL) return false;

	//InitializeSRWLock(&_usedSessionLock);



	_ListenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_ListenSocket == INVALID_SOCKET)
	{
		return -1;
	}

	int optval = 0;
	int retval = setsockopt(_ListenSocket, SOL_SOCKET, SO_RCVBUF, (char *)&optval, sizeof(optval));
	if (retval == SOCKET_ERROR)
	{
		int err = GetLastError();
		InterlockedIncrement(&_Monitor_AcceptFail);
		closesocket(_ListenSocket);
		return -1;
		//return -1;
	}

	ZeroMemory(&sockAddr, sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	sockAddr.sin_port = htons(_iListenPort);
	retval = bind(_ListenSocket, (SOCKADDR *)&sockAddr, sizeof(sockAddr));

	if (retval == SOCKET_ERROR)
	{
		return -1;
	}

	retval = listen(_ListenSocket, SOMAXCONN);
	if (retval == SOCKET_ERROR)
	{
		return -1;
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

}

bool CMMOServer::Stop(void)
{
	return true;
}

void CMMOServer::SetSessionArray(int iArrayIndex, Session *pSession)
{
	pSession = _pSessionArray[iArrayIndex];
}

unsigned __stdcall	CMMOServer::AcceptThread(void *pParam)
{
	CMMOServer *_this = (CMMOServer *)pParam;
	_this ->AcceptThread_update();
}
bool				CMMOServer::AcceptThread_update(void)
{
	CLIENT_CONNECT_INFO *connectInfo=new CLIENT_CONNECT_INFO;
	SOCKET sock;
	SOCKADDR_IN sockAddr;
	
	int addrLen;

	int sessionPos;

	WCHAR IP[16];

	while (1)
	{
		addrLen = sizeof(sockAddr);

		PRO_BEGIN(L"ACCEPT");
		connectInfo->sock = accept(_ListenSocket, (SOCKADDR *)&(sockAddr), &addrLen);
		PRO_END(L"ACCEPT");

		if (connectInfo->sock == INVALID_SOCKET)
		{
			int err = WSAGetLastError();
			InterlockedIncrement(&_Monitor_AcceptFail);
			continue;
		}
		InterlockedIncrement(&_Monitor_AcceptSocket);
		
		InetNtopW(AF_INET, &sockAddr.sin_addr, connectInfo->IP, 16);
		connectInfo->Port = sockAddr.sin_port;
		
		_AcceptSocketQueue.Enqueue(connectInfo);
	}
}

unsigned __stdcall	CMMOServer::AuthThread(void *pParam)
{
	CMMOServer *_this = (CMMOServer *)pParam;
	_this->AcceptThread_update();
}
bool				CMMOServer::AuthThread_update(void)
{
	ProcAuth_Accept();
	ProcAuth_Packet();
	OnAuth_Update();
	ProcAuth_StatusChange();

	Sleep(10);
}

void CMMOServer::ProcAuth_Accept()
{
	CLIENT_CONNECT_INFO *connectInfo;
	Session *session;
	int sessionPos;

	_AcceptSocketQueue.Dequeue(&connectInfo);

	if (connectInfo == NULL)
		return;

	_BlankSessionStack.Pop(&sessionPos);
	session = _pSessionArray[sessionPos];

	session->SetClientInfo(connectInfo);
	session->SetMode(MODE_AUTH);

	session->OnAuth_ClientJoin();

	//recv걸기

	//
}
void CMMOServer::ProcAuth_Packet()
{
	Packet *p;
	Session *session;
	for (int i = 0; i < _maxSession; i++)
	{
		session = _pSessionArray[i];
		if (session = NULL)
			continue;

		if (session->GetMode() != MODE_AUTH)
			continue;

		if (session->GetLogoutFlag())
		{
			session->SetMode(MODE_LOGOUT_IN_AUTH);
			continue;
		}

		p = session->CompletePacketQ().front();
		session->CompletePacketQ().pop();


		session->OnAuth_Packet(p);
	}
}


void CMMOServer::ProcAuth_StatusChange(void)
{
	Session *session;
	for (int i = 0; i < _maxSession; i++)
	{
		session = _pSessionArray[i];
		if (session = NULL)
			continue;

		if (session->GetMode() == MODE_LOGOUT_IN_AUTH)
		{
			session->SetMode(WAIT_LOGOUT);
		}
		else if (session->GetMode() == MODE_AUTH)
		{
			if (session->GetAuthToGameFlag())
			{
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
	_this->GameUpdateThread_update();
}

bool CMMOServer::GameUpdateThread_update(void)
{
	ProcGame_AuthToGame();
	ProcGame_Packet();
	OnAuth_Update();
	ProcGame_Logout();
	ProcGame_Release();

	//Sleep(10);
}

void CMMOServer::ProcGame_AuthToGame()
{
	Session *session;
	for (int i = 0; i < _maxSession; i++)
	{
		session = _pSessionArray[i];
		if (session = NULL)
			continue;

		if (session->GetMode() != MODE_AUTH_TO_GAME)
			continue;

		session->SetMode(MODE_GAME);

		session->OnGame_ClientJoin();
	}
}

void CMMOServer::ProcGame_Packet()
{
	Packet *p;
	Session *session;
	for (int i = 0; i < _maxSession; i++)
	{
		session = _pSessionArray[i];
		if (session = NULL)
			continue;

		if (session->GetMode() != MODE_GAME)
			continue;

		if (session->GetLogoutFlag())
		{
			session->SetMode(MODE_LOGOUT_IN_GAME);
			continue;
		}

		p = session->CompletePacketQ().front();
		session->CompletePacketQ().pop();


		session->OnGame_Packet(p);
	}
}

void CMMOServer::ProcGame_Logout()
{
	Session *session;
	for (int i = 0; i < _maxSession; i++)
	{
		session = _pSessionArray[i];
		if (session = NULL)
			continue;

		if (session->GetMode() != MODE_LOGOUT_IN_GAME)
		{
			continue;
		}

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
		if (session = NULL)
			continue;

		if (session->GetMode() != WAIT_LOGOUT)
			continue;

		session->OnGame_ClientRelease();
		
		_BlankSessionStack.Push(session->GetIndex());
	}
}

unsigned __stdcall	CMMOServer::IOCPWorkerThread(void *pParam)
{
	CMMOServer *_this = (CMMOServer *)pParam;
	_this->IOCPWorkerThread_update();
}

bool				CMMOServer::IOCPWorkerThread_update(void)
{
	int ret;
	Session *session;
	OVERLAPPED *pOverlapped;
	DWORD transferred;
	while (1)
	{
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
				result = CompleteRecvPacket(session);
				if (result != SUCCESS)
					break;
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
			session->Disconnect();
		}
	}

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

	Packet::Free(payload);

	return SUCCESS;
}

bool CMMOServer::RecvPost(Session *session, bool first = false)
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
	PRO_END(L"WSARecv");

	if (retval == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != ERROR_IO_PENDING)
		{
			if (InterlockedDecrement64(&session->IOCount()) == 0)
			{
				session->Disconnect();
			}

			return false;
		}
	}

	return true;
}

unsigned __stdcall	CMMOServer::SendThread(void *pParam)
{
	CMMOServer *_this = (CMMOServer *)pParam;
	_this->SendThread_update();
}

bool				CMMOServer::SendThread_update(void)
{
	DWORD flags = 0;
	Session *session;
	en_SESSION_MODE mode;
	while (1)
	{
		Session *session;
		int peekCnt;// = session->GetSendQ()->GetUseCount();
		WSABUF wsabuf[1024];
		Packet *peekData[1024];
		for (int i = 0; i < _maxSession; i++)
		{
			session = _pSessionArray[i];
			if (session = NULL)
				continue;

			if (!InterlockedExchange(&session->SendFlag(), true))
			{
				InterlockedExchange(&session->SendFlag(), false);
				continue;
			}

			mode = session->GetMode();
			if (mode != MODE_AUTH && mode != MODE_GAME)
			{
				InterlockedExchange(&session->SendFlag(), false);
				continue;
			}

			peekCnt = session->SendQ().GetUseCount();

			if (peekCnt == 0)
			{
				InterlockedExchange(&session->SendFlag(), false);
				continue;
			}

			peekCnt = session->SendQ().Peek(peekData, peekCnt);

			session->SetSendCnt(peekCnt);

			PRO_BEGIN(L"WSASend");
			int retval = WSASend(session->Socket(), wsabuf, peekCnt, NULL, flags, (OVERLAPPED *)&session->SendOverlap(), NULL);
			PRO_END(L"WSASend");

			if (retval == SOCKET_ERROR)
			{
				int err;
				if ((err = WSAGetLastError()) != ERROR_IO_PENDING)
				{
					if (InterlockedDecrement64(&session->IOCount()) == 0)
					{
						session->Disconnect();
					}
				}
			}
		}

		Sleep(5);
	}
}

unsigned __stdcall	CMMOServer::MonitorThread(void *pParam)
{
	CMMOServer *_this = (CMMOServer *)pParam;
	_this->MonitorThread_update();
}

bool				CMMOServer::MonitorThread_update(void)
{
}