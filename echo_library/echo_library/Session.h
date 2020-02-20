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

	BOOL RecvPost(BOOL);
	BOOL SendPost();
	BOOL Release();

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
};