#pragma once

#define PORT 6000
#define THREAD_CNT 6
#define RUNNING_CNT 3

enum TYPE { SEND = 0, RECV };

struct MyOverlapped
{
	OVERLAPPED overlap;
	TYPE type;
	VOID *session;
};

struct Session
{
	SOCKET sock;
	SOCKADDR_IN sockAddr;
	MyOverlapped sendOverlap;
	MyOverlapped recvOverlap;
	RingBuffer sendQ;
	RingBuffer recvQ;
	BOOL sendFlag;
};