#define _CRT_SECURE_NO_WARNINGS
#include "network.h"
#include "Protocol.h"
#include <map>
#include <iostream>
#include "Info.h"


SOCKET listenSock;

UINT64 g_accountID = 1;
int g_clientNo = 0;

std::map<int,LoginUser *> UserMap;
std::map<int, Account *> AccountMap;
std::map<int, Friend *> FriendMap;
std::map<int, FriendRequest *> RequestMap;

bool initNetwork()
{
	SOCKADDR_IN addr;
	ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(dfNETWORK_PORT);

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return false;

	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (listenSock == INVALID_SOCKET)
	{
		//std::cout << "socket error" << std::endl;
		system("pause");
		return false;
	}

	if (bind(listenSock, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		//std::cout << "bind error" << std::endl;
		system("pause");
		return false;
	}

	if (listen(listenSock, SOMAXCONN) == SOCKET_ERROR)
	{
		//std::cout << "listen error" << std::endl;
		system("pause");
		return false;
	}

	u_long on = 1;
	if (ioctlsocket(listenSock, FIONBIO, &on) == SOCKET_ERROR)
	{
		//std::cout << "non blocking error" << std::endl;
		system("pause");
		return false;
	}

	std::wcout.imbue(std::locale("kor")); // 이것을 추가하면 된다.
	std::wcin.imbue(std::locale("kor")); // cin은 이것을 추가
}

bool networkProcess()
{
	
	int UserIDTable[FD_SETSIZE];
	SOCKET UserSockTable[FD_SETSIZE];
	int sockCount=0;
	LoginUser *client;

	FD_SET ReadSet;
	FD_SET WriteSet;
	FD_ZERO(&ReadSet);
	FD_ZERO(&WriteSet);
	memset(UserIDTable, -1, sizeof(int) * FD_SETSIZE);
	memset(UserSockTable, INVALID_SOCKET, sizeof(DWORD) * FD_SETSIZE);

	//listen sock 삽입
	FD_SET(listenSock, &ReadSet);
	UserIDTable[sockCount] = 0;
	UserSockTable[sockCount] = listenSock;
	sockCount++;

	for (auto iter = UserMap.begin(); iter != UserMap.end();)
	{
		client = iter->second;
		
		UserIDTable[sockCount] = client->clientNo;
		//UserTable[sockCount] = client;
		UserSockTable[sockCount] = client->sock;

		FD_SET(client->sock, &ReadSet);

		if (client->SendQ.GetUseSize() > 0)
			FD_SET(client->sock, &WriteSet);

		sockCount++;
		iter++;
		if (FD_SETSIZE <= sockCount)
		{
			//select
			callSelect(UserIDTable, UserSockTable, &ReadSet, &WriteSet);
			
			FD_ZERO(&ReadSet);
			FD_ZERO(&WriteSet);
			memset(UserIDTable, -1, sizeof(DWORD) * FD_SETSIZE);
			memset(UserSockTable, INVALID_SOCKET, sizeof(DWORD) * FD_SETSIZE);
			sockCount = 0;
		}
	}

	if (sockCount > 0)
	{
		//select
		callSelect(UserIDTable, UserSockTable, &ReadSet, &WriteSet);
	}

	return true;
}

bool callSelect(int *IDTable,SOCKET *sockTable,FD_SET *ReadSet,FD_SET *WriteSet)
{
	timeval Time;
	int result;
	
	Time.tv_sec = 0;
	Time.tv_usec = 0;

	result = select(0, ReadSet, WriteSet, NULL, &Time);

	if (result > 0)
	{
		for (int i = 0; i < FD_SETSIZE; i++)
		{
			if (sockTable[i] == INVALID_SOCKET)
				continue;

			//write 체크
			if (FD_ISSET(sockTable[i], WriteSet))
			{
				procSend(IDTable[i]);
				//send
			}

			//read 체크
			if (FD_ISSET(sockTable[i], ReadSet))
			{
				//
				//if (IDTable[i] == 0)
				if(sockTable[i]==listenSock)
				{
					//연결 성공
					procAccept();
					//system("pause");
				}
				else
				{
					//recv실패시
					if (!procRecv(IDTable[i]))
					{
						closesocket(sockTable[i]);
						LoginUser *client = FindClient(IDTable[i]);
						UserMap.erase(client->clientNo);

						delete client;
						std::wcout << IDTable[i] << L"유저 접속 종료" << std::endl;
					}
					//system("pause");
					//recv
				}
			}
		}
	}
	else if (result == SOCKET_ERROR)
	{
		return false;
	}

	return true;
}

bool procAccept()
{
	LoginUser* client=new LoginUser();
	SOCKADDR_IN addr;
	int addrLen = sizeof(addr);
	//ZeroMemory(&(client->addr), sizeof(client->addr));
	
	
	client->sock = accept(listenSock, (sockaddr *)&(addr), &addrLen);
	if (client->sock == INVALID_SOCKET)
	{
		std::wcout << L"accept failed" << std::endl;
		return false;
	}
	
	UserMap.insert(std::make_pair(g_clientNo, client));
	g_clientNo++;
	//
	std::wcout << L"accept" << std::endl;

	return true;
}


bool procSend(DWORD userID)
{
	st_PACKET_HEADER header;
	LoginUser* client = FindClient(userID);
	int send_size;

	if (client == NULL)
		return false;

	client->SendQ.Peek((char *)&header, sizeof(header));

	std::wcout << L"Packet Send [UserNo:" << client->clientNo << "] [Type:" << header.wMsgType << "]" << std::endl;

	if (client->SendQ.GetUseSize() > client->SendQ.DirectDequeueSize())
	{
		send_size = send(client->sock, client->SendQ.GetReadPos(), client->SendQ.DirectDequeueSize(), 0);

		if (send_size == SOCKET_ERROR && GetLastError() != WSAEWOULDBLOCK)
		{
			return false;
		}

		client->SendQ.MoveFront(send_size);

		send_size = send(client->sock, client->SendQ.GetReadPos(), client->SendQ.GetUseSize(), 0);

		if (send_size == SOCKET_ERROR && GetLastError() != WSAEWOULDBLOCK)
		{
			return false;
		}
		client->SendQ.MoveFront(send_size);
	}
	else
	{
		send_size = send(client->sock, client->SendQ.GetReadPos(), client->SendQ.GetUseSize(), 0);

		if (send_size == SOCKET_ERROR && GetLastError() != WSAEWOULDBLOCK)
		{
			return false;
		}
		client->SendQ.MoveFront(send_size);
	}

	return true;
}

bool procRecv(DWORD userID)
{
	st_PACKET_HEADER header;
	LoginUser* client = FindClient(userID);
	int size;
	int t_size;
	Packet payLoad;

	if (client == NULL)
		return false;

	if (client->RecvQ.GetFreeSize() > client->RecvQ.DirectEnqueueSize())
	{
		size = recv(client->sock, client->RecvQ.GetWritePos(), client->RecvQ.DirectEnqueueSize(), 0);
		//t_size = size;
		if (size == SOCKET_ERROR)
		{
			//int temp = 0;
			//system("pause");
			if (GetLastError() != WSAEWOULDBLOCK)
				return false;
				//exit(-1);
		}
		else
			client->RecvQ.MoveRear(size);
		size = recv(client->sock, client->RecvQ.GetWritePos(), client->RecvQ.GetFreeSize(), 0);
		//t_size += size;
		if (size == SOCKET_ERROR)
		{ 
			//int temp = 0;
			//system("pause");
			if (GetLastError() != WSAEWOULDBLOCK)
				return false;
				//exit(-1);
		}
		else
			client->RecvQ.MoveRear(size);
	}
	else
	{
		size = recv(client->sock, client->RecvQ.GetWritePos(), client->RecvQ.DirectEnqueueSize(), 0);

		if (size == SOCKET_ERROR)
		{
			return false;
			//int temp = 0;
			//system("pause");
			//exit(-1);
		}
		client->RecvQ.MoveRear(size);

	}

	while (1)
	{
		if (sizeof(header) > client->RecvQ.GetUseSize())
			break;
		if (client->RecvQ.Peek((char*)&header, sizeof(header)) < sizeof(header))
			break;

		if (header.byCode != dfPACKET_CODE)
			return false;

		if (client->RecvQ.GetUseSize() < header.wPayloadSize)
			break;



		client->RecvQ.MoveFront(sizeof(header));
		client->RecvQ.Dequeue(payLoad, header.wPayloadSize);

		std::wcout << L"PacketRecv [UserNO:" << client->clientNo << L"] [Type:" << header.wMsgType << "]" << std::endl;

		switch (header.wMsgType)
		{
		case df_REQ_ACCOUNT_ADD:
			recvAccountADD(client, payLoad);
			break;
		case df_REQ_LOGIN:
			recvLogin(client, payLoad);
			break;
		case df_REQ_ACCOUNT_LIST:
			break;
		case df_REQ_FRIEND_LIST:
			break;
		case df_REQ_FRIEND_REQUEST_LIST:
			break;
		case df_REQ_FRIEND_REPLY_LIST:
			break;
		case df_REQ_FRIEND_REMOVE:
			break;
		default:
			return false;
			break;
		}
	}

	return true;
}

LoginUser *FindClient(int id)
{
	for (auto iter = UserMap.begin(); iter != UserMap.end(); iter++)
	{
		if (iter->second->clientNo == id)
		{
			return iter->second;
		}
	}

	return NULL;
}

Account *FindAccount(UINT64 id)
{
	for (auto iter = AccountMap.begin(); iter != AccountMap.end(); iter++)
	{
		if (iter->second->accountNo == id)
		{
			return iter->second;
		}
	}

	return NULL;
}



bool SendUnicast(LoginUser* client, st_PACKET_HEADER& header, Packet& p)
{
	if (client == NULL)
	{
		return false;
	}

	client->SendQ.Enqueue((char*)&header, sizeof(header));
	client->SendQ.Enqueue(p);

	//send(client->socket, client->SendQ.GetReadPos(), sizeof(header), NULL);
	

	return true;
}

bool SendBroadCast(LoginUser* client,st_PACKET_HEADER& header, Packet& p)
{
	for (auto iter = UserMap.begin(); iter != UserMap.end(); iter++)
	{
		if (client->clientNo != iter->second->clientNo)
		{
			SendUnicast(iter->second, header, p);
		}
	}

	return true;
}

bool SendBroadCastAll(LoginUser* client, st_PACKET_HEADER& header, Packet& p)
{
	for (auto iter = UserMap.begin(); iter != UserMap.end(); iter++)
	{
		SendUnicast(iter->second, header, p);
	}

	return true;
}

bool recvAccountADD(LoginUser *client, Packet &p)
{
	//WCHAR nick[dfNICK_MAX_LEN];
	Account *account=new Account();

	p.GetData((char *)account->ID, sizeof(WCHAR)*dfNICK_MAX_LEN);
	account->accountNo=g_accountID;
	g_accountID++;

	AccountMap.insert(std::make_pair(account->accountNo, account));

	sendAccountADD(client, account->accountNo);

	return true;
}

bool sendAccountADD(LoginUser *client, UINT64 accountNo)
{
	Packet p;
	st_PACKET_HEADER header;
	
	p << accountNo;

	header.byCode = dfPACKET_CODE;
	header.wMsgType = df_RES_ACCOUNT_ADD;
	header.wPayloadSize = p.GetDataSize();

	SendUnicast(client,header, p);

	return true;
}

bool recvLogin(LoginUser *client, Packet &p)
{
	UINT64 accountNo;
	p >> accountNo;
	Account *account = FindAccount(accountNo);

	client->account = account;

	sendLogin(client);
	
	return true;
}
bool sendLogin(LoginUser *client)
{
	Packet p;
	st_PACKET_HEADER header;
	
	if (client->account != NULL)
	{
		p << client->account->accountNo;
		p.PutData((char *)client->account->ID, sizeof(WCHAR)*dfNICK_MAX_LEN);
		std::wcout << client->account->accountNo;
	}
	else
	{
		p << (UINT64)0;
		//p.MoveWritePos(sizeof(WCHAR)*dfNICK_MAX_LEN);
	}
	

	header.byCode = dfPACKET_CODE;
	header.wMsgType = df_RES_LOGIN;
	header.wPayloadSize = p.GetDataSize();

	SendUnicast(client, header, p);

	return true;
}