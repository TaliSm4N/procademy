#pragma once
#include <Windows.h>
#include "RingBuffer.h"
#include "Protocol.h"

struct Account
{
	UINT64 accountNo;
	WCHAR ID[dfNICK_MAX_LEN];
};

struct Friend
{
	int froomAccountNo;
	int toAccountNo;
};

struct FriendRequest
{
	int froomAccountNo;
	int toAccountNo;
};

struct LoginUser
{
	int clientNo;
	Account *account;
	//int accountNo;
	int sock;
	RingBuffer SendQ;
	RingBuffer RecvQ;
};