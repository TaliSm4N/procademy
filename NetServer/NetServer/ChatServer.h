#pragma once

#define dfID_MAX_LEN 20
#define dfNICK_MAX_LEN 20
#define dfSESSION_KEY_BYTE_LEN 8
////////////////////////////////////////////////////////////////////////////
// UpdateThread 메시지
//
// OnRecv, OnClientJoin, OnClientLeave 발생시 메시지로 담아서 UpdateThread 에게 넘김
////////////////////////////////////////////////////////////////////////////
struct st_UPDATE_MESSAGE
{
	int		iMsgType;
	DWORD	SessionID;
	Packet	*pPacket;
};


////////////////////////////////////////////////////////////////////////////
// 클라이언트 컨텐츠 객체
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


	static unsigned int WINAPI UpdateThread(LPVOID lpParam);
	unsigned int UpdateThreadRun();
	//virtual
public:
	virtual bool OnConnectionRequest(WCHAR *ClientIP, int Port);
	virtual void OnRecv(DWORD sessionID, Packet *p);
	virtual void OnSend(DWORD sessionID, int sendsize);
	virtual void OnClientJoin(DWORD sessionID);
	virtual void OnClientLeave(DWORD sessionID);
	virtual void OnError(int errorcode, WCHAR *);
private:
	std::unordered_map<DWORD, st_PLAYER *> *_playerMap;
	SRWLOCK _playerMapLock;
	HANDLE _updateThread;
	DWORD _updateThreadID;
	LockFreeQueue<st_UPDATE_MESSAGE> *_msgQ;

	//섹터 체크 자료구조하나
};