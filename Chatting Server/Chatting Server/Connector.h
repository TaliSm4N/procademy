#pragma once

class ChatServer;

class Connector :public CLanClient
{
public:
	Connector();
	void Init(ChatServer *chat);

	bool ClientLogin(Packet *p);

	bool GetConnectFlag() { return _conn; }

private:
	Packet *MakeServerConnect(BYTE type);
	Packet *MakeClientLoginRes(INT64 accountNo, INT64 parameter);
public:
	virtual void OnServerJoin();
	virtual void OnServerLeave();
	virtual bool OnConnectionRequest(WCHAR *ClientIP, int Port);
	virtual void OnRecv(Packet *p);
	virtual void OnSend(int sendsize);

	virtual void OnError(int errorcode, WCHAR *);
private:
	ChatServer *_chatServer;
	bool _conn;
	//std::unordered_map<INT64, char *> keyMap;

};