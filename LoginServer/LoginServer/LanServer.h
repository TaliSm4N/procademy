#pragma once

//enum PROCRESULT { SUCCESS = 0, NONE, FAIL };


class CLanServer
{
private:
	enum PROCRESULT { SUCCESS = 0, NONE, FAIL };
public:
	CLanServer();
	bool Config(const WCHAR *configFile, const WCHAR *block);
	bool Start();
	bool Start(int port, int workerCnt, bool nagle, int maxUser,bool monitoring=true);
	void Stop();
	
	bool Disconnect(DWORD sessionID);

	bool RecvPost(LanSession *session);
	bool SendPost(LanSession *session);

	bool SendPacket(DWORD sessionID, Packet *p);

	//< Accept 후 접속처리 완료 후 호출.
	virtual void OnClientJoin(DWORD sessionID) = 0; 

	// < Release 후 호출
	virtual void OnClientLeave(DWORD sessionID) = 0;


	//< accept 직후
	//return false; 시 클라이언트 거부.
	//return true; 시 접속 허용
	virtual bool OnConnectionRequest(WCHAR *ClientIP, int Port) = 0;

	// < 패킷 수신 완료 후
	virtual void OnRecv(DWORD sessionID, Packet *p) = 0;

	// < 패킷 송신 완료 후
	virtual void OnSend(DWORD sessionID, int sendsize) = 0;

	virtual void OnError(int errorcode, WCHAR *) = 0;

	Packet *PacketAlloc() { return packetPool->Alloc(); }
	bool PacketFree(Packet *p) { return packetPool->Free(p); }

private:
	static unsigned int WINAPI AcceptThread(LPVOID lpParam);
	static unsigned int WINAPI WorkerThread(LPVOID lpParam);
	static unsigned int WINAPI MonitorThread(LPVOID lpParam);
	PROCRESULT CompleteRecvPacket(LanSession *session);
private:
	SOCKET _listenSock;
	SOCKADDR_IN _sockAddr;
	WCHAR _ip[16];
	int _port;
	int _workerCnt;
	int _activeCnt;
	LOG_LEVEL _logLevel;
	bool _nagle;
	int _maxUser;
	HANDLE _hcp;
	WSADATA _wsa;
	HANDLE _hAcceptThread;
	DWORD _dwAcceptThreadID;
	HANDLE *_hWokerThreads;
	DWORD *_dwWOrkerThreadIDs;

	bool _monitoring;
	HANDLE _hMonitorThread;
	DWORD _dwMonitorThreadID;

	SRWLOCK sessionListLock;
	DWORD _sessionCount;

	LanSession *_sessionList;
	//std::stack<DWORD> _unUsedSessionStack;
	//SRWLOCK _usedSessionLock;
	LockFreeStack<int> *_sessionIndexStack;

	MemoryPoolTLS<Packet> *packetPool;

	//getSettingInfo
public:
	DWORD GetSessionCount() const { return _sessionCount; }
	DWORD GetWorkerThreadCount() const { return _workerCnt; }
	DWORD GetMaxUser() const { return _maxUser; }
	//monitoring
public:
	LONG64 GetAcceptTotal() const { return _acceptTotal; }
	LONG64 GettAcceptTPS() const { return _acceptTPS; }
	LONG64 GetRecvPacketTPS() const { return _recvPacketTPS; }
	LONG64 GetSendPacketTPS() const { return _sendPacketTPS; }
	LONG64 GetAcceptFail() const { return _acceptFail; }
	LONG64 GetConnectionRequestFail() const { return _connectionRequestFail; }
	LONG64 GetPacketCount() const { return _packetCount; }

	//새로운 디스커넥트 관련 테스트
public:
	LanSession *GetSession(DWORD sessionID);
	void PutSession(LanSession *session);
	void ReleaseSession(LanSession *session);

	bool OldDisconnect(DWORD sessionID);

	
public://monitoring
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

public:
	LONG64 _disconnectCount;
	LONG64 _releaseCount;
	LONG64 _recvOverlap;
	LONG64 _sendOverlap;
	LONG64 _sessionGetCount;
//public:
//	bool AutoSendPacket(DWORD sessionID, PacketPtr *p);
};