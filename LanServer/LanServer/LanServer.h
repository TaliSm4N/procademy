#pragma once

enum PROCRESULT { SUCCESS = 0, NONE, FAIL };
struct LanServerHeader
{
	WORD len;
};

class CLanServer
{
public:
	CLanServer();
	bool Start(int port, int workerCnt, bool nagle, int maxUser,bool monitoring=true);
	void Stop();
	int GetSessionCount() const { return _sessionCount; }
	bool Disconnect(DWORD sessionID);

	bool RecvPost(Session *session);
	bool SendPost(Session *session);

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

		//	virtual void OnWorkerThreadBegin() = 0;                    < 워커스레드 GQCS 바로 하단에서 호출
		//	virtual void OnWorkerThreadEnd() = 0;                      < 워커스레드 1루프 종료 후

	virtual void OnError(int errorcode, WCHAR *) = 0;

public:
	DWORD GetAcceptTotal() { return _acceptTotal; }
	DWORD GettAcceptTPS() { return _acceptTPS; }
	DWORD GetRecvPacketTPS() { return _recvPacketTPS; }
	DWORD GetSendPacketTPS() { return _sendPacketTPS; }

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

	std::map<DWORD,Session *>sessionList;
	SRWLOCK sessionListLock;
	DWORD _sessionCount;

	
	
private://monitoring
	LONGLONG _acceptTotal;
	LONGLONG _acceptTPS;
	LONGLONG _recvPacketTPS;
	LONGLONG _sendPacketTPS;
	LONGLONG _recvPacketCounter;
	LONGLONG _sendPacketCounter;
	LONGLONG _packetPoolUse;
	LONGLONG _packetPoolAlloc;
};