#pragma once

class Player : public Session
{
public:
	Player();

private:
	void Auth_ReqLogin(Packet *p);
	void Game_Echo(Packet *p);

private:
	virtual void OnAuth_ClientJoin();
	virtual void OnAuth_ClientLeave();
	virtual void OnAuth_Packet(Packet *p);

	virtual void OnGame_ClientJoin();
	virtual void OnGame_ClientLeave();
	virtual void OnGame_ClientRelease();
	virtual void OnGame_Packet(Packet *p);
private:
	INT64 _AccountNo;
};