#pragma once

#define dfSECTOR_MAX 100

#define dfID_MAX_LEN 20
#define dfNICK_MAX_LEN 20
#define dfSESSION_KEY_BYTE_LEN 8
////////////////////////////////////////////////////////////////////////////
// UpdateThread �޽���
//
// OnRecv, OnClientJoin, OnClientLeave �߻��� �޽����� ��Ƽ� UpdateThread ���� �ѱ�
////////////////////////////////////////////////////////////////////////////
struct st_UPDATE_MESSAGE
{
	int		iMsgType;
	DWORD	SessionID;
	Packet	*pPacket;
};


////////////////////////////////////////////////////////////////////////////
// Ŭ���̾�Ʈ ������ ��ü
//
//
////////////////////////////////////////////////////////////////////////////
struct st_PLAYER
{
	DWORD	SessionID;

	INT64	AccountNo;
	WCHAR	szID[dfID_MAX_LEN];
	WCHAR	szNick[dfNICK_MAX_LEN];

	BYTE	SessionKey[dfSESSION_KEY_BYTE_LEN];

	short	shSectorX;
	short	shSectorY;

	ULONGLONG	LastRecvPacket;

};


class ChatServer :public CNetServer
{
public:
	ChatServer();
	bool Start(int port, int workerCnt, bool nagle, int maxUser, bool monitoring = true);
	bool ConfigStart(const WCHAR *configFile);


	static unsigned int WINAPI UpdateThread(LPVOID lpParam);
	static unsigned int WINAPI Monitor(LPVOID lpParam);
	unsigned int UpdateThreadRun();
	unsigned int MonitorThreadRun();
	//virtual
public:
	virtual bool OnConnectionRequest(WCHAR *ClientIP, int Port);
	virtual void OnRecv(DWORD sessionID, Packet *p);
	virtual void OnSend(DWORD sessionID, int sendsize);
	virtual void OnClientJoin(DWORD sessionID);
	virtual void OnClientLeave(DWORD sessionID);
	virtual void OnError(int errorcode, WCHAR *);

public:
	void ReqLogin(DWORD sessionID,Packet *p);
	void ReqSectorMove(DWORD sessionID, Packet *p);
	void ReqMessage(DWORD sessionID, Packet *p);
public:
	Packet *MakeResLogin(BYTE status, INT64 accountID);
	Packet *MakeResMoveSector(INT64 accountID, WORD x, WORD y);
	Packet *MakeResMessage(st_PLAYER *player, WORD msgLen, WCHAR *msg);

public:
	void SendUnicast(DWORD sessionID,Packet *p);
	void SendSector(int x, int y,Packet *p);
	void SendSectorAround(int x, int y, Packet *p);
private:
	std::unordered_map<DWORD, st_PLAYER *> *_playerMap;
	SRWLOCK _playerMapLock;
	HANDLE _updateThread;
	DWORD _updateThreadID;
	HANDLE _monitor;
	DWORD _monitorID;
	LockFreeQueue<st_UPDATE_MESSAGE *> *_msgQ;

	MemoryPoolTLS< st_UPDATE_MESSAGE> *_updateMsgPool;
	MemoryPoolTLS< st_PLAYER> *_playerPool;

	HANDLE _msgHandle;

	std::list<st_PLAYER *> _sector[dfSECTOR_MAX][dfSECTOR_MAX];

	//monitoring
public:

	int GetUpdateMsgPoolCount() { return _updateMsgPoolCount; }
	int GetUpdateMsgQCount() { return _updateMsgQCount; }
	int GetPlayerCount() { return _playerCount; }
	int GetPlayerPoolCount() { return _playerPoolCount; }
private:

	int _updateMsgPoolCount;
	int _updateMsgQCount;

	int _playerCount;
	int _playerPoolCount;
};