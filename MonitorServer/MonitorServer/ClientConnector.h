#pragma once

class ClientConnector :public CNetServer
{
public:
	struct Monitor
	{
		DWORD sessionID;
	};
public:
	ClientConnector();
	bool Start(int port, int workerCnt, bool nagle, int maxUser, bool monitoring = true);
	bool Start();
	bool Config(const WCHAR *configFile);

	void ClientLogin(DWORD sessionID, Packet *p);
	void SendMonitoring(BYTE ServerNo, BYTE DataType, int DataValue, int TimeStamp);
public:
	Packet *MakeLogin(BYTE stat);
public:
	void OnClientJoin(DWORD sessionID);
	void OnClientLeave(DWORD sessionID);
	bool OnConnectionRequest(WCHAR *ClientIP, int Port);
	void OnRecv(DWORD sessionID, Packet *p);
	void OnSend(DWORD sessionID, int sendsize);
	void OnError(int errorcode, WCHAR *);
private:
	std::vector<Monitor *> monitorVec;
	SRWLOCK monitorLock;
};