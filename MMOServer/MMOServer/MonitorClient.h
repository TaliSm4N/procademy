#pragma once

class MonitorClient : public CLanClient
{
public:
	MonitorClient();

	bool ClientLogin(Packet *p);

	bool GetConnectFlag() { return _conn; }

	void SendMonitoringMsg(BYTE	DataType, int DataValue, int TimeStamp);

private:
	Packet *MakeServerConnect(BYTE type);
	Packet *MakeMonitorMsg(BYTE	DataType, int DataValue, int TimeStamp);
public:
	virtual void OnServerJoin();
	virtual void OnServerLeave();
	virtual bool OnConnectionRequest(WCHAR *ClientIP, int Port);
	virtual void OnRecv(Packet *p);
	virtual void OnSend(int sendsize);

	virtual void OnError(int errorcode, WCHAR *);
private:
	bool _conn;
	//std::unordered_map<INT64, char *> keyMap;
};