#pragma once

#define WORKER_THREAD_MAX 10

enum PROCRESULT { SUCCESS = 0, NONE, FAIL };

class CMMOServer
{
public:
	CMMOServer(int iMaxSession, bool monitoring);
	virtual	~CMMOServer();

	///////////////////////////////////////////////////////////////////
	// 서버가동!
	///////////////////////////////////////////////////////////////////
	bool	Start(WCHAR *szListenIP, int iPort, int iWorkerThread, bool bEnableNagle, BYTE byPacketCode, BYTE byPacketKey);

	///////////////////////////////////////////////////////////////////
	// 서버중지!
	///////////////////////////////////////////////////////////////////
	bool	Stop(void);


	///////////////////////////////////////////////////////////////////
	// 외부에서 세션객체 연결
	///////////////////////////////////////////////////////////////////
	void	SetSessionArray(int iArrayIndex, Session *pSession);


	//////////////////////////////////////////////////////////////
	// 패킷 보내기, 전체세션 대상.
	//////////////////////////////////////////////////////////////
	void	SendPacket_GameAll(Packet *pPacket, DWORD ExcludeClientID = 0);

	//////////////////////////////////////////////////////////////
	// 패킷 보내기, 특정 클라이언트
	//////////////////////////////////////////////////////////////
	void	SendPacket(Packet *pPacket, DWORD ClientID);

private:
	PROCRESULT CompleteRecvPacket(Session *session);
	bool RecvPost(Session *session, bool first = false);

private:

	void	Error(int ErrorCode, WCHAR *szFormatStr, ...);



	///////////////////////////////////////////////////////////////////
	// Auth,Game 스레드의 처리함수.
	///////////////////////////////////////////////////////////////////
	void ProcAuth_Accept(void);
	void ProcAuth_Packet(void);
	void ProcAuth_StatusChange(void);

	void ProcGame_AuthToGame(void);
	void ProcGame_Packet(void);
	void ProcGame_Logout(void);

	void ProcGame_Release(void);


private:

	// AUTH 모드 업데이트 이벤트 로직처리부
	virtual	void	OnAuth_Update(void) = 0;

	// GAME 모드 업데이트 이벤트 로직처리부
	virtual	void	OnGame_Update(void) = 0;

	virtual void	OnError(int iErrorCode, WCHAR *szError) = 0;	// 에러 


private:

	bool _monitoring; //monitoring 스레드 작동여부
	int _maxSession;

	//----------------------------------------------------------------------------
	// Listen Socket
	//----------------------------------------------------------------------------
	SOCKET		_ListenSocket;

	

	bool	_bEnableNagle;
	int		_iWorkerThread;

	WCHAR	_szListenIP[16];
	int		_iListenPort;
	BYTE	_byPacketCode;

	//----------------------------------------------------------------------------
	// Accept 부
	//----------------------------------------------------------------------------
	HANDLE		_hAcceptThread;

	LockFreeQueue<CLIENT_CONNECT_INFO *>	_AcceptSocketQueue;			// 신규접속 Socket 큐
	MemoryPool<CLIENT_CONNECT_INFO>	_MemoryPool_ConnectInfo;

	//----------------------------------------------------------------------------
	// Auth 부
	//----------------------------------------------------------------------------
	HANDLE		_hAuthThread;

	LockFreeStack<int>	_BlankSessionStack;			// 빈 세션 index

	//----------------------------------------------------------------------------
	// GameUpdate 부
	//---------------------------------------------------------------------------
	HANDLE		_hGameUpdateThread;

	//----------------------------------------------------------------------------
	// IOCP 부
	//----------------------------------------------------------------------------
	HANDLE		_hIOCPWorkerThread[WORKER_THREAD_MAX];
	HANDLE		_hIOCP;

	//----------------------------------------------------------------------------
	// Send 부
	//----------------------------------------------------------------------------
	HANDLE		_hSendThread;


	Session	**_pSessionArray;

	//----------------------------------------------------------------------------
	// Monitoring 부
	//----------------------------------------------------------------------------
	HANDLE		_hMonitorThread;

	//-----------------------------------------------------------
	/////////////////////////////////////////////////////////////
	// 스레드 함수
	/////////////////////////////////////////////////////////////
	//-----------------------------------------------------------
	static unsigned __stdcall	AcceptThread(void *pParam);
	bool				AcceptThread_update(void);

	static unsigned __stdcall	AuthThread(void *pParam);
	bool				AuthThread_update(void);

	static unsigned __stdcall	GameUpdateThread(void *pParam);
	bool				GameUpdateThread_update(void);


	static unsigned __stdcall	IOCPWorkerThread(void *pParam);
	bool				IOCPWorkerThread_update(void);

	static unsigned __stdcall	SendThread(void *pParam);
	bool				SendThread_update(void);

	static unsigned __stdcall	MonitorThread(void *pParam);
	bool				MonitorThread_update(void);


public:

	int GetWorkerThreadCount() { return _iWorkerThread; }
	int GetMaxUser() { return _maxSession; }

	long		_Monitor_AcceptSocket;
	long		_Monitor_AcceptFail;
	long		_Monitor_SessionAllMode;
	long		_Monitor_SessionAuthMode;
	long		_Monitor_SessionGameMode;

	long		_Monitor_Counter_AuthUpdate;
	long		_Monitor_Counter_GameUpdate;
	long		_Monitor_Counter_Accept;	//AcceptTPS
	long		_Monitor_Counter_PacketProc;	//RecvPacketTPS
	long		_Monitor_Counter_PacketSend;	//SendPacketTPS
	long		_Monitor_Counter_Packet;

	
private:
	long		_Monitor_PacketProc;	//RecvPacket
	long		_Monitor_PacketSend;	//SendPacket
	long		_Counter_AuthUpdate;
	long		_Counter_GameUpdate;

	//debug용 함수
private:
	long GET_AUTHCOUNT();
	long GET_GAMECOUNT();
	long GET_USERCOUNT();
};