#pragma once

//class ServerConnector;

enum Status {LOGOUT_MODE = 0,LOGIN_MODE,LOGIN_TO_CHAT,CHAT};

struct Waiter
{
	DWORD SessionID;
	INT64 AccountNo;
	WCHAR id[20];
	WCHAR nick[20];
	DWORD startTime;
	DWORD endTime;
	int count;// 여러 서버 연결시 체크용도
	//char SessionKey[64];
};

class LoginServer :public CNetServer
{
public:
	LoginServer();
	bool Start(int port, int workerCnt, bool nagle, int maxUser, bool monitoring = true);
	bool Start();
	bool Config(const WCHAR *configFile, const WCHAR *block);
	bool LoginRequest(DWORD sessionID,Packet *p);


	bool ClientLoginRes_ss(INT64 accountNo, INT64 parameter, int type);

	void ServerDownMsg(BYTE type);

public:
	ServerConnector &GetConnector() { return _connecter; }
public:
	virtual bool OnConnectionRequest(WCHAR *ClientIP, int Port);
	virtual void OnRecv(DWORD sessionID, Packet *p);
	virtual void OnSend(DWORD sessionID, int sendsize);
	virtual void OnClientJoin(DWORD sessionID);
	virtual void OnClientLeave(DWORD sessionID);
	virtual void OnError(int errorcode, WCHAR *);
	
public:
	int GetLoginWait() { return _WaiterMap.size(); }

	long successCnt;
	long successCntTPS;


	INT64 loginAll;
	unsigned long loginMax;
	unsigned long loginMin;

private:
	Packet *MakeLoginResponse( INT64 accountNo, BYTE status, WCHAR *id, WCHAR *nick);
private:
	DBConnect accountDB;
	SRWLOCK dbLock;
	std::unordered_map<DWORD, Waiter *> _WaiterMap;
	SRWLOCK waiterLock;
	ServerConnector _connecter;
	MemoryPoolTLS<Waiter> _waiterPool;

	WCHAR GameIP[16];
	USHORT GamePort;
	WCHAR ChatIP[16];
	USHORT ChatPort;

};