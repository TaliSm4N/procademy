#pragma once

enum PROCRESULT { SUCCESS = 0, NONE, FAIL };


class CNetServer
{
public:
	CNetServer();
	bool ConfigStart(const WCHAR *configFile);
	bool Config(const WCHAR *configFile, const WCHAR *block);
	bool Start();
	bool Start(int port, int workerCnt, bool nagle, int maxUser,bool monitoring=true);
	void Stop();
	
	bool Disconnect(DWORD sessionID);

	bool RecvPost(Session *session);
	bool SendPost(Session *session);

	bool SendPacket(DWORD sessionID, Packet *p);

	//< Accept �� ����ó�� �Ϸ� �� ȣ��.
	virtual void OnClientJoin(DWORD sessionID) = 0; 

	// < Release �� ȣ��
	virtual void OnClientLeave(DWORD sessionID) = 0;


	//< accept ����
	//return false; �� Ŭ���̾�Ʈ �ź�.
	//return true; �� ���� ���
	virtual bool OnConnectionRequest(WCHAR *ClientIP, int Port) = 0;

	// < ��Ŷ ���� �Ϸ� ��
	virtual void OnRecv(DWORD sessionID, Packet *p) = 0;

	// < ��Ŷ �۽� �Ϸ� ��
	virtual void OnSend(DWORD sessionID, int sendsize) = 0;

		//	virtual void OnWorkerThreadBegin() = 0;                    < ��Ŀ������ GQCS �ٷ� �ϴܿ��� ȣ��
		//	virtual void OnWorkerThreadEnd() = 0;                      < ��Ŀ������ 1���� ���� ��

	virtual void OnError(int errorcode, WCHAR *) = 0;

	Packet *PacketAlloc() { return packetPool->Alloc(); }
	bool PacketFree(Packet *p) { return packetPool->Free(p); }

private:
	static unsigned int WINAPI AcceptThread(LPVOID lpParam);
	static unsigned int WINAPI WorkerThread(LPVOID lpParam);
	static unsigned int WINAPI MonitorThread(LPVOID lpParam);
	PROCRESULT CompleteRecvPacket(Session *session);
private:
	SOCKET _listenSock;
	SOCKADDR_IN _sockAddr;
	int _port;
	int _workerCnt;
	int _activeCnt;
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

	Session *_sessionList;
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

	//���ο� ��Ŀ��Ʈ ���� �׽�Ʈ
public:
	Session *GetSession(DWORD sessionID);
	void PutSession(Session *session);
	void ReleaseSession(Session *session);

	
//monitoring
public:
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