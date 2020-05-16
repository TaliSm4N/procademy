#pragma once

#define WORKER_THREAD_MAX 10

enum PROCRESULT { SUCCESS = 0, NONE, FAIL };

class CMMOServer
{
public:
	CMMOServer(int iMaxSession, bool monitoring);
	virtual	~CMMOServer();

	///////////////////////////////////////////////////////////////////
	// ��������!
	///////////////////////////////////////////////////////////////////
	bool	Start(WCHAR *szListenIP, int iPort, int iWorkerThread, bool bEnableNagle, BYTE byPacketCode, BYTE byPacketKey);

	///////////////////////////////////////////////////////////////////
	// ��������!
	///////////////////////////////////////////////////////////////////
	bool	Stop(void);


	///////////////////////////////////////////////////////////////////
	// �ܺο��� ���ǰ�ü ����
	///////////////////////////////////////////////////////////////////
	void	SetSessionArray(int iArrayIndex, Session *pSession);


	//////////////////////////////////////////////////////////////
	// ��Ŷ ������, ��ü���� ���.
	//////////////////////////////////////////////////////////////
	void	SendPacket_GameAll(Packet *pPacket, DWORD ExcludeClientID = 0);

	//////////////////////////////////////////////////////////////
	// ��Ŷ ������, Ư�� Ŭ���̾�Ʈ
	//////////////////////////////////////////////////////////////
	void	SendPacket(Packet *pPacket, DWORD ClientID);

private:
	PROCRESULT CompleteRecvPacket(Session *session);
	bool RecvPost(Session *session, bool first = false);

private:

	void	Error(int ErrorCode, WCHAR *szFormatStr, ...);



	///////////////////////////////////////////////////////////////////
	// Auth,Game �������� ó���Լ�.
	///////////////////////////////////////////////////////////////////
	void ProcAuth_Accept(void);
	void ProcAuth_Packet(void);
	void ProcAuth_StatusChange(void);

	void ProcGame_AuthToGame(void);
	void ProcGame_Packet(void);
	void ProcGame_Logout(void);

	void ProcGame_Release(void);


private:

	// AUTH ��� ������Ʈ �̺�Ʈ ����ó����
	virtual	void	OnAuth_Update(void) = 0;

	// GAME ��� ������Ʈ �̺�Ʈ ����ó����
	virtual	void	OnGame_Update(void) = 0;

	virtual void	OnError(int iErrorCode, WCHAR *szError) = 0;	// ���� 


private:

	bool _monitoring; //monitoring ������ �۵�����
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
	// Accept ��
	//----------------------------------------------------------------------------
	HANDLE		_hAcceptThread;

	LockFreeQueue<CLIENT_CONNECT_INFO *>	_AcceptSocketQueue;			// �ű����� Socket ť
	MemoryPool<CLIENT_CONNECT_INFO>	_MemoryPool_ConnectInfo;

	//----------------------------------------------------------------------------
	// Auth ��
	//----------------------------------------------------------------------------
	HANDLE		_hAuthThread;

	LockFreeStack<int>	_BlankSessionStack;			// �� ���� index

	//----------------------------------------------------------------------------
	// GameUpdate ��
	//---------------------------------------------------------------------------
	HANDLE		_hGameUpdateThread;

	//----------------------------------------------------------------------------
	// IOCP ��
	//----------------------------------------------------------------------------
	HANDLE		_hIOCPWorkerThread[WORKER_THREAD_MAX];
	HANDLE		_hIOCP;

	//----------------------------------------------------------------------------
	// Send ��
	//----------------------------------------------------------------------------
	HANDLE		_hSendThread;


	Session	**_pSessionArray;

	//----------------------------------------------------------------------------
	// Monitoring ��
	//----------------------------------------------------------------------------
	HANDLE		_hMonitorThread;

	//-----------------------------------------------------------
	/////////////////////////////////////////////////////////////
	// ������ �Լ�
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

	//debug�� �Լ�
private:
	long GET_AUTHCOUNT();
	long GET_GAMECOUNT();
	long GET_USERCOUNT();
};