#pragma once

class LoginServer;

class ServerConnector :public CLanServer
{
private:
	struct ConnectedServer
	{
		DWORD sessionID;
		BYTE type;
		WCHAR IP[16];
		int port;
	};
public:
	ServerConnector();
	void Init(LoginServer *login);

	bool ClientLoginReq_ss(INT64 accountNo, char *sessionKey, INT64 parameter, int type);

public:
	virtual bool OnConnectionRequest(WCHAR *ClientIP, int Port);
	virtual void OnRecv(DWORD sessionID, Packet *p);
	virtual void OnSend(DWORD sessionID, int sendsize);
	virtual void OnClientJoin(DWORD sessionID);
	virtual void OnClientLeave(DWORD sessionID);
	virtual void OnError(int errorcode, WCHAR *);
private:
	bool ServerLogin(DWORD sessionID,Packet *p);
	
	bool ClientLoginRes(DWORD sessionID, Packet *p);
	
	
	ConnectedServer *GetServer(int type);

private:
	LoginServer *_loginServer;

	std::unordered_map<DWORD,ConnectedServer *> _serverMap;
	SRWLOCK _serverMapLock;
};