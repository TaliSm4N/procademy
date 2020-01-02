#define _CRT_SECURE_NO_WARNINGS
#include "network.h"
#include "Protocol.h"
#include <map>
#include "info.h"
#include <iostream>
#include "Protocol.h"

std::map<DWORD, UserInfo*> UserMap;
std::map<DWORD, RoomInfo*> RoomMap;

SOCKET listenSock;
DWORD g_UserID = 1;
DWORD g_RoomID = 1;

bool initNetwork(int PORT)
{
	SOCKADDR_IN addr;
	ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(PORT);

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

	//UserInfo* listenUser = new UserInfo();
	//listenUser->socket = listenSock;
	//listenUser->addr = addr;
	//listenUser->UserID = 0;
	//UserMap.insert(std::make_pair(listenUser->UserID, listenUser));
}

bool networkProcess()
{
	UserInfo* client;
	DWORD UserIDTable[FD_SETSIZE];
	//UserInfo* UserTable[FD_SETSIZE];
	SOCKET UserSockTable[FD_SETSIZE];
	int sockCount=0;

	FD_SET ReadSet;
	FD_SET WriteSet;
	FD_ZERO(&ReadSet);
	FD_ZERO(&WriteSet);
	memset(UserIDTable, -1, sizeof(DWORD) * FD_SETSIZE);
	//memset(UserTable, -1, sizeof(UserInfo *) * FD_SETSIZE);
	memset(UserSockTable, INVALID_SOCKET, sizeof(DWORD) * FD_SETSIZE);

	//listen sock 삽입
	FD_SET(listenSock, &ReadSet);
	UserIDTable[sockCount] = 0;
	UserSockTable[sockCount] = listenSock;
	sockCount++;

	for (auto iter = UserMap.begin(); iter != UserMap.end();)
	{
		client = iter->second;
		
		UserIDTable[sockCount] = client->UserID;
		//UserTable[sockCount] = client;
		UserSockTable[sockCount] = client->socket;

		FD_SET(client->socket, &ReadSet);

		if (client->SendQ.GetUseSize() > 0)
			FD_SET(client->socket, &WriteSet);

		sockCount++;
		iter++;
		if (FD_SETSIZE <= sockCount)
		{
			//select
			callSelect(UserIDTable, UserSockTable, &ReadSet, &WriteSet);
			//callSelect(UserTable, UserSockTable, &ReadSet, &WriteSet);
			
			FD_ZERO(&ReadSet);
			FD_ZERO(&WriteSet);
			memset(UserIDTable, -1, sizeof(DWORD) * FD_SETSIZE);
			//memset(UserTable, -1, sizeof(UserInfo*) * FD_SETSIZE);
			memset(UserSockTable, INVALID_SOCKET, sizeof(DWORD) * FD_SETSIZE);
			sockCount = 0;
		}
	}

	if (sockCount > 0)
	{
		//select
		//callSelect(UserTable, UserSockTable, &ReadSet, &WriteSet);
		callSelect(UserIDTable, UserSockTable, &ReadSet, &WriteSet);
	}

	return true;
}

bool callSelect(DWORD *IDTable,SOCKET *sockTable,FD_SET *ReadSet,FD_SET *WriteSet)
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
						UserInfo *client = FindClient(IDTable[i]);
						UserMap.erase(client->UserID);

						if (client->RoomID != 0)
						{
							sendRoomLeave(client);
						}

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
	UserInfo* client=new UserInfo();
	int addrLen = sizeof(client->addr);
	//ZeroMemory(&(client->addr), sizeof(client->addr));
	
	
	client->socket = accept(listenSock, (sockaddr *)&(client->addr), &addrLen);
	if (client->socket == INVALID_SOCKET)
	{
		std::wcout << L"accept failed" << std::endl;
		return false;
	}

	client->RoomID = 0;
	client->UserID = 0;
	
	UserMap.insert(std::make_pair(g_UserID, client));
	//
	std::wcout << L"accept" << std::endl;

	return true;
}


bool procSend(DWORD userID)
{
	st_PACKET_HEADER header;
	UserInfo* client = FindClient(userID);
	int send_size;

	if (client == NULL)
		return false;

	client->SendQ.Peek((char *)&header, sizeof(header));

	std::wcout << L"Packet Send [UserNo:" << client->UserID << "] [Type:" << header.wMsgType << "]" << std::endl;

	if (client->SendQ.GetUseSize() > client->SendQ.DirectDequeueSize())
	{
		send_size = send(client->socket, client->SendQ.GetReadPos(), client->SendQ.DirectDequeueSize(), 0);

		if (send_size == SOCKET_ERROR && GetLastError() != WSAEWOULDBLOCK)
		{
			return false;
		}

		client->SendQ.MoveFront(send_size);

		send_size = send(client->socket, client->SendQ.GetReadPos(), client->SendQ.GetUseSize(), 0);

		if (send_size == SOCKET_ERROR && GetLastError() != WSAEWOULDBLOCK)
		{
			return false;
		}
		client->SendQ.MoveFront(send_size);
	}
	else
	{
		send_size = send(client->socket, client->SendQ.GetReadPos(), client->SendQ.GetUseSize(), 0);

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
	UserInfo* client = FindClient(userID);
	int size;
	int t_size;
	Packet payLoad;

	if (client == NULL)
		return false;

	if (client->RecvQ.GetFreeSize() > client->RecvQ.DirectEnqueueSize())
	{
		size = recv(client->socket, client->RecvQ.GetWritePos(), client->RecvQ.DirectEnqueueSize(), 0);
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
		size = recv(client->socket, client->RecvQ.GetWritePos(), client->RecvQ.GetFreeSize(), 0);
		//t_size += size;
		if (size == SOCKET_ERROR)
		{
			//int temp = 0;
			//system("pause");
			if (GetLastError() != WSAEWOULDBLOCK)
				exit(-1);
		}
		else
			client->RecvQ.MoveRear(size);
	}
	else
	{
		size = recv(client->socket, client->RecvQ.GetWritePos(), client->RecvQ.DirectEnqueueSize(), 0);

		if (size == SOCKET_ERROR)
		{
			//int temp = 0;
			//system("pause");
			exit(-1);
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

		if (!getCheckSum(header, payLoad))
		{
			system("pause");
			exit(-1);
		}

		std::wcout << L"PacketRecv [UserNO:" << client->UserID << L"] [Type:" << header.wMsgType << "]" << std::endl;

		switch (header.wMsgType)
		{
		case df_REQ_LOGIN:
			recvLogin(client, payLoad);
			break;
		case df_REQ_CHAT:
			recvChat(client, payLoad);
			break;
		case df_REQ_ROOM_CREATE:
			recvMakeRoom(client, payLoad,header.wPayloadSize-2);
			break;
		case df_REQ_ROOM_ENTER:
			recvEnterRoom(client, payLoad);
			break;
		case df_REQ_ROOM_LEAVE:
			recvRoomLeave(client);
			break;
		case df_REQ_ROOM_LIST:
			recvRoomList(client);
			break;
		default:
			return false;
			break;
		}
	}

	return true;
}

UserInfo* FindClient(DWORD userID)
{
	for (auto iter = UserMap.begin(); iter != UserMap.end(); iter++)
	{
		if (iter->second->UserID == userID)
		{
			return iter->second;
		}
	}

	return NULL;
}
RoomInfo* FindRoom(DWORD roomID)
{
	for (auto iter = RoomMap.begin(); iter != RoomMap.end(); iter++)
	{
		if (iter->second->RoomID == roomID)
		{
			return iter->second;
		}
	}

	return NULL;
}

bool SendUnicast(UserInfo* client, st_PACKET_HEADER& header, Packet& p)
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

bool SendBroadCast(UserInfo* client,st_PACKET_HEADER& header, Packet& p)
{
	for (auto iter = UserMap.begin(); iter != UserMap.end(); iter++)
	{
		if (client->UserID != iter->second->UserID)
		{
			SendUnicast(iter->second, header, p);
		}
	}

	return true;
}

bool SendBroadCastAll(UserInfo* client, st_PACKET_HEADER& header, Packet& p)
{
	for (auto iter = UserMap.begin(); iter != UserMap.end(); iter++)
	{
		SendUnicast(iter->second, header, p);
	}

	return true;
}

bool SendBroadCast_Room(UserInfo* client,RoomInfo *room, st_PACKET_HEADER& header, Packet& p)
{
	for (auto iter = room->UserList.begin(); iter != room->UserList.end(); iter++)
	{
		if (client->UserID != (*iter)->UserID)
		{
			SendUnicast(*iter, header, p);
		}
	}

	return true;
}

bool SendBroadCastAll_Room(UserInfo* client, RoomInfo *room, st_PACKET_HEADER& header, Packet& p)
{
	for (auto iter = room->UserList.begin(); iter != room->UserList.end(); iter++)
	{
		SendUnicast(*iter, header, p);
	}

	return true;
}

bool recvLogin(UserInfo* client, Packet& p)
{
	WCHAR nickname[MAX_NICK_LEN];
	p.GetData((char *)nickname, sizeof(WCHAR)*MAX_NICK_LEN);

	if (client->UserID != 0)
	{
		sendLogin(client, df_RESULT_LOGIN_ETC, 0);
		closesocket(client->socket);
		UserMap.erase(client->UserID);
		delete client;
		return false;
	}

	for (auto iter = UserMap.begin(); iter != UserMap.end(); iter++)
	{
		if (wcscmp(iter->second->nickName, nickname) == 0)
		{
			sendLogin(client, df_RESULT_LOGIN_DNICK, 0);
			closesocket(client->socket);
			UserMap.erase(client->UserID);
			delete client;
			return false;
		}
	}

	wcscpy(client->nickName, nickname);
	client->UserID = g_UserID;
	

	sendLogin(client, df_RESULT_LOGIN_OK, g_UserID);
	g_UserID++;
	
	std::wcout << nickname<<L" 님이 입장함"<<std::endl;
	
	return true;
}

bool recvMakeRoom(UserInfo* client, Packet& p,WORD len)
{
	WORD size;
	p >> size;
	WCHAR roomName[MAX_ROOM_LEN];
	p.GetData((char *)roomName, sizeof(WCHAR)*MAX_ROOM_LEN);

	if (client->RoomID != 0)
	{
		sendMakeRoom(client, df_RESULT_ROOM_CREATE_ETC, NULL);
		return false;
	}

	for (auto iter = RoomMap.begin(); iter != RoomMap.end(); iter++)
	{
		if (len / sizeof(WCHAR) == iter->second->titleLen)
		{
			bool same = true;

			for (int i = 0; i < iter->second->titleLen; i++)
			{
				if (roomName[i] != iter->second->Title[i])
				{
					same = false;
					break;
				}
			}

			if (same)
			{
				sendMakeRoom(client, df_RESULT_ROOM_CREATE_DNICK, NULL);
				return false;
			}

			//if (wcscmp(iter->second->Title, roomName) == 0)
			//{
			//	sendMakeRoom(client, df_RESULT_ROOM_CREATE_DNICK, NULL);
			//	return false;
			//}
		}
	}

	RoomInfo *room = new RoomInfo();
	room->RoomID = g_RoomID;
	memcpy(room->Title, roomName,len);
	room->titleLen = len / sizeof(WCHAR);
	//room->UserList.push_back(client);
	//client->RoomID = g_RoomID;
	g_RoomID++;

	RoomMap.insert(std::make_pair(room->RoomID, room));

	sendMakeRoom(client, df_RESULT_ROOM_CREATE_OK, room);
	//sendRoomList(client);

	for (int i = 0; i < room->titleLen; i++)
	{
		std::wcout << room->Title[i];
	}

	std::wcout << L"방을 생성함" << std::endl;

	return true;
}

bool recvEnterRoom(UserInfo* client, Packet& p)
{
	DWORD roomID;
	RoomInfo *room;
	p >> roomID;
	room = FindRoom(roomID);

	if (room==NULL)
	{
		sendEnterRoom(client, df_RESULT_ROOM_ENTER_NOT, NULL);
		return false;
	}
	else if(client->RoomID!=0)
	{
		sendEnterRoom(client, df_RESULT_ROOM_ENTER_ETC, NULL);
		return false;
	}

	room->UserList.push_back(client);
	client->RoomID = roomID;

	sendEnterRoom(client, df_RESULT_ROOM_ENTER_OK, room);

	for (int i = 0; i < room->titleLen; i++)
	{
		std::wcout << room->Title[i];
	}

	std::wcout << L"방에 입장함" << std::endl;

	return true;
}

bool recvRoomList(UserInfo* client)
{
	sendRoomList(client);

	return true;
}

bool recvRoomLeave(UserInfo* client)
{
	if (client->RoomID == 0)
		return false;
	sendRoomLeave(client);

	return true;
}
bool recvChat(UserInfo* client, Packet& p)
{
	RoomInfo *room=FindRoom(client->RoomID);
	WORD size;
	WCHAR msg[256];
	p >> size;
	p.GetData((char *)msg, size);

	sendChat(client, room, msg,size);

	return true;
}

bool sendChat(UserInfo *client, RoomInfo *room, WCHAR *msg,WORD len)
{
	Packet p;
	st_PACKET_HEADER header;
	p << client->UserID << len;
	p.PutData((char *)msg, len);

	header.byCode = dfPACKET_CODE;
	header.byCheckSum = setCheckSum(p, df_RES_CHAT);
	header.wMsgType = df_RES_CHAT;
	header.wPayloadSize = p.GetDataSize();

	//SendBroadCast(client, header, p);
	SendBroadCast_Room(client, room, header, p);
	//SendBroadCastAll_Room(client, room, header, p);

	return true;
}


bool sendRoomLeave(UserInfo *client)
{
	Packet p;
	Packet p2;
	RoomInfo *room = FindRoom(client->RoomID);
	st_PACKET_HEADER header;
	p << client->UserID;

	header.byCode = dfPACKET_CODE;
	header.byCheckSum = setCheckSum(p, df_RES_ROOM_LEAVE);
	header.wMsgType = df_RES_ROOM_LEAVE;
	header.wPayloadSize = p.GetDataSize();

	SendBroadCastAll_Room(client, room, header, p);

	for (auto iter = room->UserList.begin(); iter != room->UserList.end(); iter++)
	{
		if ((*iter)->UserID == client->UserID)
		{
			iter=room->UserList.erase(iter);
			break;
		}
	}

	if (room->UserList.size() == 0)
	{
		p2 << room->RoomID;
		header.byCode = dfPACKET_CODE;
		header.byCheckSum = setCheckSum(p2, df_RES_ROOM_DELETE);
		header.wMsgType = df_RES_ROOM_DELETE;
		header.wPayloadSize = p2.GetDataSize();
		SendBroadCastAll(client, header, p2);

		RoomMap.erase(room->RoomID);
		delete room;
	}

	client->RoomID = 0;

	return true;
}

bool sendRoomList(UserInfo *client)
{
	Packet p;
	st_PACKET_HEADER header;
	WORD size = RoomMap.size();
	p << size;
	std::wcout << size << std::endl;
	for (auto iter = RoomMap.begin(); iter != RoomMap.end(); iter++)
	{
		std::wcout<< L"{" << std::endl;
		p << iter->second->RoomID;
		std::wcout << L"\t" << iter->second->RoomID << std::endl;
		p << (WORD)(iter->second->titleLen * sizeof(WCHAR));
		std::wcout << L"\t" << (WORD)(iter->second->titleLen * sizeof(WCHAR)) << std::endl;
		p.PutData((char *)iter->second->Title, sizeof(WCHAR)*iter->second->titleLen);
		iter->second->Title[iter->second->titleLen] = '\0';
		std::wcout << L"\t" << iter->second->Title << std::endl<<std::endl;

		p << (BYTE)iter->second->UserList.size();
		std::wcout << L"\t" << iter->second->UserList.size() << std::endl;

		for (auto uIter = iter->second->UserList.begin(); uIter != iter->second->UserList.end(); uIter++)
		{
			std::wcout << L"\t" << L"{" << std::endl;
			p.PutData((char *)(*uIter)->nickName, sizeof((*uIter)->nickName));
			std::wcout << L"\t" << L"\t" << (*uIter)->nickName << std::endl;
			std::wcout << L"\t" << L"}" << std::endl;
		}
		std::wcout << L"}" << std::endl;
	}
	
	header.byCode = dfPACKET_CODE;
	header.byCheckSum = setCheckSum(p, df_RES_ROOM_LIST);
	header.wMsgType = df_RES_ROOM_LIST;
	header.wPayloadSize = p.GetDataSize();

	SendUnicast(client, header, p);

	return true;
}

bool sendEnterRoom(UserInfo *client, BYTE result, RoomInfo *room)
{
	Packet p;
	Packet p2;
	st_PACKET_HEADER header;
	p << result;

	if (result == df_RESULT_ROOM_ENTER_OK)
	{
		p << room->RoomID;
		p << (WORD)(room->titleLen * sizeof(WCHAR));
		p.PutData((char *)(room->Title), room->titleLen * sizeof(WCHAR));
		p << (BYTE)(room->UserList.size());
		for (auto iter = room->UserList.begin(); iter != room->UserList.end(); iter++)
		{
			p.PutData((char *)(*iter)->nickName, sizeof((*iter)->nickName));
			p << (*iter)->UserID;
		}
	}

	header.byCode = dfPACKET_CODE;
	header.byCheckSum = setCheckSum(p, df_RES_ROOM_ENTER);
	header.wMsgType = df_RES_ROOM_ENTER;
	header.wPayloadSize = p.GetDataSize();

	SendUnicast(client, header, p);

	if (room == NULL)
		return true;

	p2.PutData((char *)client->nickName, sizeof(client->nickName));
	p2 << client->UserID;
	header.byCode = dfPACKET_CODE;
	header.byCheckSum = setCheckSum(p2, df_RES_USER_ENTER);
	header.wMsgType = df_RES_USER_ENTER;
	header.wPayloadSize = p2.GetDataSize();

	SendBroadCast_Room(client, room, header, p2);

	return true;
}

bool sendLogin(UserInfo *client, BYTE result,DWORD ID)
{
	Packet p;
	st_PACKET_HEADER header;
	p << result;
	p << ID;
	
	header.byCode = dfPACKET_CODE;
	header.byCheckSum=setCheckSum(p, df_RES_LOGIN);
	header.wMsgType = df_RES_LOGIN;
	header.wPayloadSize = p.GetDataSize();

	SendUnicast(client, header, p);

	return true;
}

bool sendMakeRoom(UserInfo *client,BYTE result, RoomInfo *room)
{
	Packet p;
	st_PACKET_HEADER header;
	p << result;
	if (room != NULL)
	{
		p << room->RoomID;
		p << (WORD)(room->titleLen*sizeof(WCHAR));
		p.PutData((char *)(room->Title), room->titleLen * sizeof(WCHAR));
	}
	else
	{
		p << (DWORD)0;
		p << (WORD)0;
	}
	

	header.byCode = dfPACKET_CODE;
	header.byCheckSum = setCheckSum(p, df_RES_ROOM_CREATE);
	header.wMsgType = df_RES_ROOM_CREATE;
	header.wPayloadSize = p.GetDataSize();

	SendBroadCastAll(client, header, p);

	return true;
}

BYTE setCheckSum(Packet &p, WORD type)
{
	int size = p.GetDataSize();
	BYTE *ptr = (BYTE *)p.GetBufferPtr();
	int checkSum = type;
	for (int i = 0; i < size; i++)
	{
		checkSum += *ptr;
		ptr++;
	}

	return (BYTE)(checkSum % 256);
}

bool getCheckSum(st_PACKET_HEADER &header,Packet &p)
{
	int checkSum = setCheckSum(p, header.wMsgType);

	if (header.byCheckSum != checkSum)
	{
		return false;
	}
	else
	{
		return true;
	}
}