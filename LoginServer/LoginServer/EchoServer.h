#pragma once

class EchoServer : public CMMOServer
{
public:
	EchoServer(int iMaxSession, bool monitoring = true);
private:
	void	OnAuth_Update(void);
	void	OnGame_Update(void);
	void	OnError(int iErrorCode, WCHAR *szError);
private:
	Player *player;
};