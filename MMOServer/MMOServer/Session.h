#pragma once

class CMMOServer;

enum en_SESSION_MODE {MODE_NONE=0,MODE_AUTH,MODE_AUTH_TO_GAME,MODE_GAME,MODE_LOGOUT_IN_AUTH,MODE_LOGOUT_IN_GAME,WAIT_LOGOUT};


class Session
{
public:
	friend class CMMOServer;
public:
	void SendPacket(Packet *);
	void Disconnect();
	void Reset();
	void Logout();
public:
	void SetClientInfo(CLIENT_CONNECT_INFO *clientInfo) { _ClientInfo = *clientInfo; }
	void SetMode(en_SESSION_MODE mode) { _Mode = mode;  _bLogoutFlag = false; _bAuthToGameFlag = false;  }
	void SetSendCnt(int cnt) { _iSendPacketCnt = cnt; }
	VOID SetIndex(int index) { iArrayIndex = index; }
	void SetLogoutFlag(bool b) { _bLogoutFlag = b; }
	void SetAuthToGameFlag(bool b) { _bAuthToGameFlag = b; }

	en_SESSION_MODE GetMode() { return _Mode; }
	bool GetLogoutFlag() { return _bLogoutFlag; }
	bool GetAuthToGameFlag() { return _bAuthToGameFlag; }
	int GetIndex() { return iArrayIndex; }
	int GetSendCnt() { return _iSendPacketCnt; }

	RingBuffer &RecvQ() { return _RecvQ; }
	LockFreeQueue<Packet *> &SendQ() { return _SendQ; }
	//MyQueue<Packet *> &CompletePacketQ() { return _CompletePacket; }
	LockFreeQueue<Packet *> &CompletePacketQ() { return _CompletePacket; }
	LONG &SendFlag() { return _lSendIO; }
	OVERLAPPED &RecvOverlap() { return _RecvOverlapped; }
	OVERLAPPED &SendOverlap() { return _SendOverlapped; }
	LONG64 &IOCount() { return _IOCount; }
	SOCKET &Socket() { return _ClientInfo.sock; }

	bool ValidMode(en_SESSION_MODE mode);
public:
	virtual void OnAuth_ClientJoin();
	virtual void OnAuth_ClientLeave();
	virtual void OnAuth_Packet(Packet *p);

	virtual void OnGame_ClientJoin();
	virtual void OnGame_ClientLeave();
	virtual void OnGame_ClientRelease();
	virtual void OnGame_Packet(Packet *p);
private:
	void CloseSocket();
	
protected:
	bool				_bLogoutFlag;
	bool				_bAuthToGameFlag;
private:
	en_SESSION_MODE			_Mode;		// 세션의 상태모드
	int						iArrayIndex;	// Session 배열의 자기 인덱스

	CLIENT_CONNECT_INFO		_ClientInfo;	// IP,Port,Socket,ClientID 정보

	RingBuffer				_RecvQ;
	LockFreeQueue<Packet *>	_SendQ;

	//일반 큐로 수정 예정
	//MyQueue<Packet *>	_CompletePacket;
	LockFreeQueue<Packet *> _CompletePacket;

	OVERLAPPED			_RecvOverlapped;
	OVERLAPPED			_SendOverlapped;

	int				_iSendPacketCnt;
	int				_iSendPacketSize;

	LONG				_lSendIO;
	LONG64				_IOCount;

	SOCKET _closeSock;

	//debug용
public:
	int logSend;
	int logIOCP;
	int logRecv;
	int logAccept;
	int Senderr;
	int Recverr;

	SOCKET recvSock;
	SOCKET sendSock;

};