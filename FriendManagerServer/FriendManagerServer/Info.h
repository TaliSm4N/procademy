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
	UINT64 fromAccountNo;
	Account *fromAccount;
	Account *toAccount;
};

struct FriendRequest
{
	UINT64 fromAccountNo;
	Account *fromAccount;
	Account *toAccount;
};

struct LoginUser
{
	int clientNo;
	Account *account;
	//int accountNo;
	int sock;
	RingBuffer SendQ;
	RingBuffer RecvQ;

	~LoginUser()
	{
		SendQ.~RingBuffer();
		RecvQ.~RingBuffer();
		
	};
};
