#pragma once




class CLanClient
{
private:
	enum PROCRESULT { SUCCESS = 0, NONE, FAIL };
public:
	CLanClient();
	bool initConnectInfo(const WCHAR *ip, int port);
	bool Config(const WCHAR *configFile, const WCHAR *block);
	bool Start();
	bool Start(int workerCnt, bool nagle,bool monitoring=true);
	void Stop();
	
	bool Disconnect();

	bool RecvPost();
	bool SendPost();

	bool SendPacket(Packet *p);

	//< Accept �� ����ó�� �Ϸ� �� ȣ��.
	virtual void OnServerJoin() = 0; 

	// < Release �� ȣ��
	virtual void OnServerLeave() = 0;


	//< accept ����
	//return false; �� Ŭ���̾�Ʈ �ź�.
	//return true; �� ���� ���
	virtual bool OnConnectionRequest(WCHAR *ClientIP, int Port) = 0;

	// < ��Ŷ ���� �Ϸ� ��
	virtual void OnRecv(Packet *p) = 0;

	// < ��Ŷ �۽� �Ϸ� ��
	virtual void OnSend(int sendsize) = 0;

	virtual void OnError(int errorcode, WCHAR *) = 0;

	Packet *PacketAlloc() { return packetPool->Alloc(); }
	bool PacketFree(Packet *p) { return packetPool->Free(p); }

private:
	static unsigned int WINAPI WorkerThread(LPVOID lpParam);
	unsigned int WINAPI WorkerThread_update();
	unsigned int WINAPI MonitorThread_update();
	static unsigned int WINAPI MonitorThread(LPVOID lpParam);
	PROCRESULT CompleteRecvPacket();

	bool Connect();
private:
	SOCKET _sock;
	SOCKADDR_IN _sockAddr;
	WCHAR _ip[16];
	int _port;
	int _workerCnt;
	int _activeCnt;
	bool _nagle;
	HANDLE _hcp;
	WSADATA _wsa;
	HANDLE *_hWokerThreads;
	DWORD *_dwWOrkerThreadIDs;

	bool _monitoring;
	HANDLE _hMonitorThread;
	DWORD _dwMonitorThreadID;

	//SRWLOCK sessionListLock;
	//DWORD _sessionCount;

	//std::stack<DWORD> _unUsedSessionStack;
	//SRWLOCK _usedSessionLock;

	MemoryPoolTLS<Packet> *packetPool;

	//getSettingInfo
public:
	DWORD GetWorkerThreadCount() const { return _workerCnt; }
	//monitoring
public:
	LONG64 GetAcceptTotal() const { return _acceptTotal; }
	LONG64 GettAcceptTPS() const { return _acceptTPS; }
	LONG64 GetRecvPacketTPS() const { return _recvPacketTPS; }
	LONG64 GetSendPacketTPS() const { return _sendPacketTPS; }
	LONG64 GetAcceptFail() const { return _acceptFail; }
	LONG64 GetConnectionRequestFail() const { return _connectionRequestFail; }
	LONG64 GetPacketCount() const { return _packetCount; }

	//���ο� ��Ŀ��Ʈ ���� �׽�Ʈ
public:
	//Session *GetSession(DWORD sessionID);
	//void PutSession(Session *session);
	//void ReleaseSession(Session *session);

	//���� ��ü
private:
	OVERLAPPED sendOverlap;
	OVERLAPPED recvOverlap;
	LockFreeQueue<Packet *> *sendQ;
	RingBuffer recvQ;
	CHAR sendFlag;
	int sendPacketCnt;
	LONG IOCount;
	
private://monitoring
	LONG64 _acceptTotal;
	LONG64 _acceptTPS;
	LONG64 _recvPacketTPS;
	LONG64 _sendPacketTPS;
	LONG64 _recvPacketCounter;
	LONG64 _sendPacketCounter;
	LONG64 _packetPoolUse;
	LONG64 _packetPoolAlloc;
	LONG64 _acceptFail;
	LONG64 _connectionRequestFail;
	LONG64 _packetCount;
//public:
//	bool AutoSendPacket(DWORD sessionID, PacketPtr *p);
};