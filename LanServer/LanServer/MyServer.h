#pragma once



class CMyServer :public CLanServer
{
public:
	CMyServer();
	virtual void OnRecv(DWORD sessionID, Packet *p);
	virtual void OnSend(DWORD sessionID, int sendsize);
	virtual void OnClientJoin(DWORD sessionID);
	virtual void OnClientLeave(DWORD sessionID);
	virtual void OnError(int errorcode, WCHAR *);

	bool Echo(LONGLONG sessionID, Packet &p);
private:
	std::map < DWORD, Player *> playerList;
	SRWLOCK playerListLock;
};