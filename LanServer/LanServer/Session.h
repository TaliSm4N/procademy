#pragma once

enum TYPE { SEND = 0, RECV };

struct MyOverlapped
{
	OVERLAPPED overlap;
	TYPE type;
};

class Session
{
public:
	Session(SOCKET s, SOCKADDR_IN &sAddr,LONGLONG ID);
	~Session();

	DWORD &GetIOCount() { return IOCount; }
	RingBuffer &GetRecvQ() { return recvQ; }
	RingBuffer &GetSendQ() { return sendQ; }
	CHAR &GetSendFlag() { return sendFlag; }
	SOCKET GetSocket() { return sock; }
	BOOL &GetSocketActive() { return sockActive; }
	LONGLONG GetID() { return sessionID; }
	MyOverlapped &GetSendOverlap() { return sendOverlap; }
	MyOverlapped &GetRecvOverlap() { return recvOverlap; }

	BOOL RecvPost();
	BOOL SendPost();
	BOOL Release();
	void Lock();
	void Unlock();

public:
	//test¿ë
	bool recvOn;
	bool sendOn;

private:
	SOCKET sock;
	LONGLONG sessionID;
	SOCKADDR_IN sockAddr;
	MyOverlapped sendOverlap;
	MyOverlapped recvOverlap;
	RingBuffer sendQ;
	RingBuffer recvQ;
	DWORD IOCount;
	CHAR sendFlag;
	BOOL sockActive;
	SRWLOCK sessionLock;
};