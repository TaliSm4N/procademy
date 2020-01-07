#define _CRT_SECURE_NO_WARNINGS
#include "network.h"
#include "Protocol.h"
#include <map>
#include <list>
#include <iostream>
#include "Info.h"
#pragma comment(lib, "winmm.lib")


SOCKET listenSock;

UINT64 g_accountID = 1;
int g_clientNo = 0;

std::list<LoginUser *>UserMap;
//std::map<int,LoginUser *> UserMap;
std::map<UINT64, Account *> AccountMap;
std::multimap<UINT64, Friend *> FriendMap;
std::multimap<UINT64, FriendRequest *> RequestMap;

DWORD recvCount = 0;
DWORD clientCount = 0;

Packet payLoad;

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

	//std::wcout.imbue(std::locale("kor")); // 이것을 추가하면 된다.
	//std::wcin.imbue(std::locale("kor")); // cin은 이것을 추가

	timeBeginPeriod(1);
}

bool networkProcess()
{
	static DWORD tickTime = 0;
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
		//client = iter->second;
		client = *iter;
		
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

	if (tickTime + 1000 < timeGetTime())
	{
		tickTime = timeGetTime();
		std::cout << "Clinet : " << clientCount << " recvMessage : " << recvCount << "/s" << std::endl;
		recvCount = 0;
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
						
						UserMap.remove(client);
						//UserMap.erase(client->clientNo);
						delete client;

						std::cout << IDTable[i] << "유저 접속 종료" << std::endl;
						clientCount--;
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
	LoginUser* client;// = new LoginUser();
	SOCKADDR_IN addr;
	int addrLen = sizeof(addr);
	//ZeroMemory(&(client->addr), sizeof(client->addr));
	static int count = 0;
	while (1)
	{
		client = new LoginUser();
		client->sock = accept(listenSock, (sockaddr *)&(addr), &addrLen);
		client->clientNo = g_clientNo;
		if (client->sock == INVALID_SOCKET)
		{
			delete client;
			//std::wcout << L"accept failed" << std::endl;
			return false;
		}

		//UserMap.insert(std::make_pair(g_clientNo, client));
		UserMap.push_back(client);
		g_clientNo++;
		//
		//std::wcout << L"accept"<<client->sock<<":"<<count << std::endl;

		clientCount++;
		count++;
	}

	

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

	//std::wcout << L"Packet Send [UserNo:" << client->clientNo << "] [Type:" << header.wMsgType << "]" << std::endl;

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
	//Packet payLoad;
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
			if (GetLastError() != WSAEWOULDBLOCK)
				return false;
			//int temp = 0;
			//system("pause");
			//exit(-1);
		}
		else
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

		if (client->RecvQ.GetUseSize() < header.wPayloadSize+sizeof(header))
			break;


		client->RecvQ.MoveFront(sizeof(header));
		client->RecvQ.Dequeue(payLoad, header.wPayloadSize);

		//std::wcout << L"PacketRecv [UserNO:" << client->clientNo << L"] [Type:" << header.wMsgType << "]" << std::endl;
		recvCount++;
		switch (header.wMsgType)
		{
		case df_REQ_ACCOUNT_ADD:
			recvAccountADD(client, payLoad);
			break;
		case df_REQ_LOGIN:
			recvLogin(client, payLoad);
			break;
		case df_REQ_ACCOUNT_LIST:
			recvAccountList(client);
			break;
		case df_REQ_FRIEND_LIST:
			recvFriendList(client);
			break;
		case df_REQ_FRIEND_REQUEST_LIST:
			recvRequestList(client);
			break;
		case df_REQ_FRIEND_REPLY_LIST:
			recvReplyList(client);
			break;
		case df_REQ_FRIEND_REMOVE:
			recvRemove(client, payLoad);
			break;
		case df_REQ_FRIEND_REQUEST:
			recvRequest(client, payLoad);
			break;
		case df_REQ_FRIEND_CANCEL:
			recvRequestCancel(client, payLoad);
			break;
		case df_REQ_FRIEND_DENY:
			recvRequestDeny(client, payLoad);
			break;
		case df_REQ_FRIEND_AGREE:
			recvAgree(client, payLoad);
		case df_REQ_STRESS_ECHO:
			recvStress(client, payLoad);
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
		if ((*iter)->clientNo == id)
		{
			return *iter;
		}
		//if (iter->second->clientNo == id)
		//{
		//	return iter->second;
		//}
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
	client->SendQ.Enqueue(payLoad);

	//send(client->socket, client->SendQ.GetReadPos(), sizeof(header), NULL);
	payLoad.Clear();

	return true;
}

bool SendBroadCast(LoginUser* client,st_PACKET_HEADER& header, Packet& p)
{
	for (auto iter = UserMap.begin(); iter != UserMap.end(); iter++)
	{
		if (client->clientNo != (*iter)->clientNo)
		{
			SendUnicast(*iter, header, p);
		}
		//if (client->clientNo != iter->second->clientNo)
		//{
		//	SendUnicast(iter->second, header, p);
		//}
	}

	return true;
}

bool SendBroadCastAll(LoginUser* client, st_PACKET_HEADER& header, Packet& p)
{
	for (auto iter = UserMap.begin(); iter != UserMap.end(); iter++)
	{
		SendUnicast(*iter, header, payLoad);
		//SendUnicast(iter->second, header, p);
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
	Account *account = AccountMap.find(accountNo)->second;//FindAccount(accountNo);

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
		std::cout << client->account->accountNo;
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

bool recvAccountList(LoginUser *client)
{
	sendAccountList(client);

	return true;
}

bool sendAccountList(LoginUser *client)
{
	Packet p;
	st_PACKET_HEADER header;

	p << (UINT)AccountMap.size();

	for (auto iter = AccountMap.begin(); iter != AccountMap.end(); iter++)
	{
		p << iter->second->accountNo;
		p.PutData((char *)iter->second->ID, sizeof(WCHAR)*dfNICK_MAX_LEN);
	}

	header.byCode = dfPACKET_CODE;
	header.wMsgType = df_RES_ACCOUNT_LIST;
	header.wPayloadSize = p.GetDataSize();

	SendUnicast(client, header, p);

	return true;
}

bool recvFriendList(LoginUser *client)
{
	sendFriendList(client);

	return true;
}

bool sendFriendList(LoginUser *client)
{
	Packet p;
	st_PACKET_HEADER header;
	UINT count=0;

	for (auto iter = FriendMap.begin(); iter != FriendMap.end(); iter++)
	{
		if (iter->second->fromAccountNo == client->account->accountNo)
		{
			count++;
		}
	}
	p << count;

	for (auto iter = FriendMap.begin(); iter != FriendMap.end(); iter++)
	{
		if (iter->second->fromAccountNo == client->account->accountNo)
		{
			p << iter->second->toAccount->accountNo;
			p.PutData((char *)iter->second->toAccount->ID, sizeof(WCHAR)*dfNICK_MAX_LEN);
		}
	}

	header.byCode = dfPACKET_CODE;
	header.wMsgType = df_RES_FRIEND_LIST;
	header.wPayloadSize = p.GetDataSize();

	SendUnicast(client, header, p);

	return true;
}

bool recvRequestList(LoginUser *client)
{
	sendRequestList(client);

	return true;
}

bool sendRequestList(LoginUser *client)
{
	Packet p;
	st_PACKET_HEADER header;
	UINT count = 0;

	for (auto iter = RequestMap.begin(); iter != RequestMap.end(); iter++)
	{
		if (iter->second->fromAccountNo == client->account->accountNo)
		{
			count++;
		}
	}
	p << count;

	for (auto iter = RequestMap.begin(); iter != RequestMap.end(); iter++)
	{
		if (iter->second->fromAccountNo == client->account->accountNo)
		{
			p << iter->second->toAccount->accountNo;
			p.PutData((char *)iter->second->toAccount->ID, sizeof(WCHAR)*dfNICK_MAX_LEN);
		}
	}

	header.byCode = dfPACKET_CODE;
	header.wMsgType = df_RES_FRIEND_REQUEST_LIST;
	header.wPayloadSize = p.GetDataSize();

	SendUnicast(client, header, p);

	return true;
}

bool recvReplyList(LoginUser *client)
{
	sendReplyList(client);

	return true;
}

bool sendReplyList(LoginUser *client)
{
	Packet p;
	st_PACKET_HEADER header;
	UINT count = 0;

	for (auto iter = RequestMap.begin(); iter != RequestMap.end(); iter++)
	{
		if (iter->second->toAccount->accountNo == client->account->accountNo)
		{
			count++;
		}
	}
	p << count;

	for (auto iter = RequestMap.begin(); iter != RequestMap.end(); iter++)
	{
		if (iter->second->toAccount->accountNo == client->account->accountNo)
		{
			p << iter->second->fromAccountNo;
			p.PutData((char *)iter->second->fromAccount->ID, sizeof(WCHAR)*dfNICK_MAX_LEN);
		}
	}

	header.byCode = dfPACKET_CODE;
	header.wMsgType = df_RES_FRIEND_REPLY_LIST;
	header.wPayloadSize = p.GetDataSize();

	SendUnicast(client, header, p);

	return true;
}

bool recvRemove(LoginUser *client, Packet &p)
{
	UINT64 accountNo;
	p >> accountNo;

	Friend *friendFrom=NULL;
	Friend *friendTo=NULL;
	for (auto iter = FriendMap.begin(); iter != FriendMap.end(); iter++)
	{
		if (iter->second->fromAccountNo == client->account->accountNo&&iter->second->toAccount->accountNo == accountNo)
		{
			friendFrom = iter->second;
		}

		if (iter->second->fromAccountNo == accountNo && iter->second->toAccount->accountNo == client->account->accountNo)
		{
			friendTo = iter->second;
		}
	}

	if (friendFrom != NULL && friendTo != NULL)
	{
		FriendMap.erase(friendTo->fromAccountNo);
		FriendMap.erase(friendFrom->fromAccountNo);

		delete friendTo;
		delete friendFrom;

		sendRemove(client, accountNo, df_RESULT_FRIEND_REMOVE_OK);
	}
	else if(friendFrom==NULL||friendTo==NULL)
	{
		sendRemove(client, accountNo, df_RESULT_FRIEND_REMOVE_FAIL);
	}
	else
	{
		sendRemove(client, accountNo, df_RESULT_FRIEND_REMOVE_NOTFRIEND);
	}

	return true;

}

bool sendRemove(LoginUser *client, UINT64 accountNo, BYTE result)
{
	Packet p;
	st_PACKET_HEADER header;

	p << accountNo << result;

	header.byCode = dfPACKET_CODE;
	header.wMsgType = df_RES_FRIEND_REMOVE;
	header.wPayloadSize = payLoad.GetDataSize();

	SendUnicast(client, header, p);

	return true;
}

bool recvRequest(LoginUser *client, Packet &p)
{
	UINT64 accountNo;
	p >> accountNo;

	if (AccountMap.find(accountNo) == AccountMap.end())
	{
		sendRequest(client, accountNo, df_RESULT_FRIEND_REQUEST_NOTFOUND);
		return true;
	}

	for (auto iter = FriendMap.begin(); iter != FriendMap.end(); iter++)
	{
		if (client->account->accountNo == iter->second->fromAccountNo&&accountNo == iter->second->toAccount->accountNo)
		{
			sendRequest(client, accountNo, df_RESULT_FRIEND_REQUEST_AREADY);
		}
	}
	FriendRequest *fromRequest = new FriendRequest();

	fromRequest->fromAccount = client->account;
	fromRequest->fromAccountNo = fromRequest->fromAccount->accountNo;
	fromRequest->toAccount = AccountMap.find(accountNo)->second;

	RequestMap.insert(std::make_pair(fromRequest->fromAccountNo, fromRequest));

	sendRequest(client, accountNo, df_RESULT_FRIEND_REQUEST_OK);
}

bool sendRequest(LoginUser *client, UINT64 accountNo, BYTE result)
{
	Packet p;
	st_PACKET_HEADER header;
	payLoad << accountNo << result;

	header.byCode = dfPACKET_CODE;
	header.wMsgType = df_RES_FRIEND_REQUEST;
	header.wPayloadSize = p.GetDataSize();

	SendUnicast(client, header, payLoad);

	return true;
}

bool recvRequestCancel(LoginUser *client, Packet &p)
{
	UINT accountNo;
	payLoad >> accountNo;

	if (AccountMap.find(accountNo) == AccountMap.end())
	{
		sendRequestCancel(client, accountNo, df_RESULT_FRIEND_CANCEL_NOTFRIEND);
		return true;
	}

	for (auto iter = RequestMap.begin(); iter != RequestMap.end(); iter++)
	{
		if (iter->second->fromAccountNo == client->account->accountNo&&iter->second->toAccount->accountNo == accountNo)
		{
			delete iter->second;

			RequestMap.erase(iter);

			sendRequestCancel(client, accountNo, df_RESULT_FRIEND_CANCEL_OK);

			return true;
		}
	}

	sendRequestCancel(client, accountNo, df_RESULT_FRIEND_CANCEL_FAIL);

	return true;
}

bool sendRequestCancel(LoginUser *client, UINT64 accountNo, BYTE result)
{
	Packet p;
	st_PACKET_HEADER header;
	payLoad << accountNo << result;

	header.byCode = dfPACKET_CODE;
	header.wMsgType = df_RES_FRIEND_CANCEL;
	header.wPayloadSize = payLoad.GetDataSize();

	SendUnicast(client, header, payLoad);

	return true;
}

bool recvRequestDeny(LoginUser *client, Packet &p)
{
	UINT accountNo;
	payLoad >> accountNo;

	if (AccountMap.find(accountNo) == AccountMap.end())
	{
		sendRequestDeny(client, accountNo, df_RESULT_FRIEND_CANCEL_NOTFRIEND);
		return true;
	}

	for (auto iter = RequestMap.begin(); iter != RequestMap.end(); iter++)
	{
		if (iter->second->fromAccountNo == accountNo && iter->second->toAccount->accountNo == client->account->accountNo)
		{
			delete iter->second;
			RequestMap.erase(iter);

			sendRequestDeny(client, accountNo, df_RESULT_FRIEND_DENY_OK);

			return true;
		}
	}

	sendRequestDeny(client, accountNo, df_RESULT_FRIEND_DENY_FAIL);
}

bool sendRequestDeny(LoginUser *client, UINT64 accountNo, BYTE result)
{
	//Packet p;
	st_PACKET_HEADER header;
	payLoad << accountNo << result;

	header.byCode = dfPACKET_CODE;
	header.wMsgType = df_RES_FRIEND_DENY;
	header.wPayloadSize = payLoad.GetDataSize();

	SendUnicast(client, header, payLoad);

	return true;
}

bool recvAgree(LoginUser *client, Packet &p)
{
	UINT accountNo;
	payLoad >> accountNo;

	if (AccountMap.find(accountNo) == AccountMap.end())
	{
		sendAgree(client, accountNo, df_RESULT_FRIEND_AGREE_NOTFRIEND);
		return true;
	}

	for (auto iter = RequestMap.begin(); iter != RequestMap.end(); iter++)
	{
		if (iter->second->fromAccountNo == accountNo && iter->second->toAccount->accountNo == client->account->accountNo)
		{
			Friend *fromFriend = new Friend();
			Friend *toFriend = new Friend();

			fromFriend->fromAccount = iter->second->fromAccount;
			fromFriend->fromAccountNo = iter->second->fromAccountNo;
			fromFriend->toAccount = iter->second->toAccount;
			
			toFriend->fromAccount = fromFriend->toAccount;
			toFriend->fromAccountNo = toFriend->fromAccount->accountNo;
			toFriend->toAccount = fromFriend->fromAccount;

			FriendMap.insert(std::make_pair(fromFriend->fromAccountNo, fromFriend));
			FriendMap.insert(std::make_pair(toFriend->fromAccountNo, toFriend));

			delete iter->second;
			RequestMap.erase(iter);

			sendAgree(client, accountNo, df_RESULT_FRIEND_AGREE_OK);

			return true;
		}
	}
	sendAgree(client, accountNo, df_RESULT_FRIEND_AGREE_FAIL);
}

bool sendAgree(LoginUser *client, UINT64 accountNo, BYTE result)
{
	//Packet p;
	st_PACKET_HEADER header;
	payLoad << accountNo << result;

	header.byCode = dfPACKET_CODE;
	header.wMsgType = df_RES_FRIEND_AGREE;
	header.wPayloadSize = payLoad.GetDataSize();

	SendUnicast(client, header, payLoad);

	return true;
}

bool recvStress(LoginUser *client, Packet &p)
{
	//sendStress(client, p);

	st_PACKET_HEADER header;

	header.byCode = dfPACKET_CODE;
	header.wMsgType = df_RES_STRESS_ECHO;
	header.wPayloadSize = payLoad.GetDataSize();

	SendUnicast(client, header, payLoad);

	return true;
}

bool sendStress(LoginUser *client, Packet &p)
{
	

	return true;
}