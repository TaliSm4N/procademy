#pragma once



class CMyClient :public CLanClient
{
public:
	CMyClient();
	virtual bool OnConnectionRequest(WCHAR *ClientIP, int Port);
	virtual void OnRecv(Packet *p);
	virtual void OnSend(int sendsize);
	virtual void OnServerJoin();
	virtual void OnServerLeave();
	virtual void OnError(int errorcode, WCHAR *);

	bool Echo(Packet *p);
private:

};