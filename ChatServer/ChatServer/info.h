#pragma once
#include <Windows.h>
#include "RingBuffer.h"
#include <list>

#define MAX_NICK_LEN 15
#define MAX_ROOM_LEN 256

#define MAX_USER 1024
#define MAX_ROOM 1024

struct UserInfo
{
	SOCKET socket;
	SOCKADDR_IN addr;
	RingBuffer SendQ;
	RingBuffer RecvQ;

	DWORD UserID;
	DWORD RoomID;
	WCHAR nickName[MAX_NICK_LEN];
};

struct RoomInfo
{
	DWORD RoomID;
	WCHAR Title[MAX_ROOM_LEN];
	WORD titleLen;
	std::list<UserInfo*> UserList;
};