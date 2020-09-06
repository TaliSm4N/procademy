#pragma once

class MonitorServer : public CLanServer
{
private:
	struct Server
	{
		DWORD id;
		int serverNo;
	};

public:
	MonitorServer();
	bool Start(int port, int workerCnt, bool nagle, int maxUser, bool monitoring = true);
	bool Start();
	bool Config(const WCHAR *configFile);

public:
	void MonitorLogin(DWORD sessionID, Packet *p);
	void MonitorDataUpdate(DWORD sessionID, Packet *p);
public:
	virtual bool OnConnectionRequest(WCHAR *ClientIP, int Port);
	virtual void OnRecv(DWORD sessionID, Packet *p);
	virtual void OnSend(DWORD sessionID, int sendsize);
	virtual void OnClientJoin(DWORD sessionID);
	virtual void OnClientLeave(DWORD sessionID);
	virtual void OnError(int errorcode, WCHAR *);
private:
	DBConnectTLS LogDB;

	SRWLOCK _serverLock;
	std::unordered_map<DWORD,Server *> _serverMap;

	ClientConnector _connector;
private:
};