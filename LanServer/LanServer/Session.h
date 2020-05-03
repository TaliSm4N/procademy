#pragma once

enum TYPE { SEND = 0, RECV };

struct MyOverlapped
{
	OVERLAPPED overlap;
	TYPE type;
};

struct IOChecker
{
	LONG64 IOCount;
	LONG64 releaseFlag;
};

class Session
{	
public:
	Session(SOCKET s, SOCKADDR_IN &sAddr,DWORD ID);
	Session();

	void SetSessionInfo(SOCKET s, SOCKADDR_IN &sAddr, DWORD ID);
	~Session();

	LONG64 &GetIOCount() { return IOBlock->IOCount; }
	IOChecker *GetIOBlock() { return IOBlock; }
	RingBuffer &GetRecvQ() { return recvQ; }
	LONG64 &GetReleaseFlag() { return IOBlock->releaseFlag; }
	//IOChecker *GetIOCheckerPtr() { return &_IOChecker; }
	//RingBuffer &GetSendQ() { return sendQ; }

	LockFreeQueue<Packet *> *GetSendQ() { return sendQ; }

	CHAR &GetSendFlag() { return sendFlag; }
	SOCKET &GetSocket() { return sock; }
	BOOL &GetSocketActive() { return sockActive; }
	DWORD &GetID() { return sessionID; }
	MyOverlapped &GetSendOverlap() { return sendOverlap; }
	MyOverlapped &GetRecvOverlap() { return recvOverlap; }

	BOOL Release();
	void Lock();
	void Unlock();

	void SetSendPacketCnt(int cnt) { sendPacketCnt = cnt; }
	int GetSendPacketCnt() { return sendPacketCnt; }

	bool acceptCheck;
private:
	SOCKET sock;
	DWORD sessionID;
	SOCKADDR_IN sockAddr;
	MyOverlapped sendOverlap;
	MyOverlapped recvOverlap;
	//RingBuffer sendQ;
	LockFreeQueue<Packet *> *sendQ;
	RingBuffer recvQ;
	CHAR sendFlag;
	BOOL sockActive;
	SRWLOCK sessionLock;
	int sendPacketCnt;

	//DWORD IOCount;
	//bool releaseFlag;
	IOChecker *IOBlock;

	

	//자동화 테스트용
//public:
//	RingBuffer &GetAutoSendQ() { return autoSendQ; }
//private:
//	RingBuffer autoSendQ;

};