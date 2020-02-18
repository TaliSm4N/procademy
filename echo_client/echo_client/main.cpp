#pragma comment(lib,"ws2_32")
#define _CRT_SECURE_NO_WARNINGS
#define UNICODE
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include "Packet.h"
#include "RingBuffer.h"

#define SERVERPORT 9000

#define TEST_SIZE 81 * 3 + 1

struct st_PACKET_HEADER
{
	BYTE	byCode;

	WORD	wMsgType;
	WORD	wPayloadSize;
};

struct User
{
	SOCKET sock;
	char buf[81 * 3 + 1];
	char recvbuf[81 * 3 + 1];
	int buf_size;
	bool recv;
	RingBuffer RecvQ;
	bool valid;
	int echo;
};
char testStr[81 * 3 + 1] = "1234567890 abcdefghijklmnopqrstuvwxyz 1234567890 abcdefghijklmnopqrstuvwxyz 123451234567890 abcdefghijklmnopqrstuvwxyz 1234567890 abcdefghijklmnopqrstuvwxyz 123451234567890 abcdefghijklmnopqrstuvwxyz 1234567890 abcdefghijklmnopqrstuvwxyz 12345";
User user[TEST_SIZE];

int main()
{
	WSADATA wsa;
	SOCKET sock;

	int retval;

	char buf[512];

	//家南 烹脚 何
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));

	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &serveraddr.sin_addr.s_addr);
	serveraddr.sin_port = htons(SERVERPORT);
	
	sock = socket(AF_INET, SOCK_STREAM, 0);

	/*
	for (int i = 0; i < TEST_SIZE; i++)
	{
		user[i].sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		//connectTry++;
		if (user[i].sock == INVALID_SOCKET)
		{
			//connectFail++;
			user[i].valid = false;
			continue;
			//std::cout << "肋给等 家南" << std::endl;
			//system("pause");
			//return 1;
		}

		if ((int)connect(user[i].sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR)
		{
			//connectFail++;
			user[i].valid = false;
			continue;
			//std::cout << "connect 俊矾" << std::endl;
			//system("pause");
			//return 1;
		}

		u_long on = 1;
		if (ioctlsocket(user[i].sock, FIONBIO, &on) == SOCKET_ERROR)
		{
			//std::cout << "non blocking error" << std::endl;
			system("pause");
			return false;
		}

		user[i].recv = true;
		user[i].valid = true;
	}
	int cnt=0;
	while (1)
	{
		
		for (int i = 0; i < TEST_SIZE; i++)
		{
			if (!user[i].valid)
				continue;

			int size;
			if (user[i].recv)
			{
				//int temp = rand() % (81 * 3 + 1);
				int temp = i;//rand() % (81 * 3 + 1);
				memcpy(user[i].buf, testStr, temp);
				user[i].buf[temp] = '\0';
				//header.byCode = dfPACKET_CODE;
				//header.wMsgType = df_REQ_STRESS_ECHO;


				//send(user[i].sock, (char *)&header, sizeof(header), 0);

				//p << temp;
				//p.PutData(user[i].buf, temp);
				//header.wPayloadSize = p.GetBufferSize();
				//send(user[i].sock, p.GetBufferPtr(), p.GetBufferSize(), 0);
				send(user[i].sock, user[i].buf, temp, 0);

				//p.Clear();
				user[i].recv = false;
				user[i].buf_size = temp;
				//user[i].echo = timeGetTime();
			}
			else
			{

				if (user[i].RecvQ.GetFreeSize() > user[i].RecvQ.DirectEnqueueSize())
				{
					size = recv(user[i].sock, user[i].RecvQ.GetWritePos(), user[i].RecvQ.DirectEnqueueSize(), 0);
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
						user[i].RecvQ.MoveRear(size);
					size = recv(user[i].sock, user[i].RecvQ.GetWritePos(), user[i].RecvQ.GetFreeSize(), 0);
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
						user[i].RecvQ.MoveRear(size);
				}
				else
				{
					size = recv(user[i].sock, user[i].RecvQ.GetWritePos(), user[i].RecvQ.DirectEnqueueSize(), 0);

					if (size == SOCKET_ERROR)
					{
						if (GetLastError() != WSAEWOULDBLOCK)
							return false;
						//int temp = 0;
						//system("pause");
						//exit(-1);
					}
					else
						user[i].RecvQ.MoveRear(size);
				}

				while (1)
				{
					//if (sizeof(header) > user[i].RecvQ.GetUseSize())
					//	break;
					//if (user[i].RecvQ.Peek((char*)&header, sizeof(header)) < sizeof(header))
					//	break;

					//if (header.byCode != dfPACKET_CODE)
					//{
					//	user[i].valid = false;
					//	closesocket(user[i].sock);
					//	errorCount++;
					//	break;
					//}

					//if (user[i].RecvQ.GetUseSize() < header.wPayloadSize + sizeof(header))
					//	break;

					//user[i].RecvQ.MoveFront(sizeof(header));
					//user[i].RecvQ.Dequeue(p, header.wPayloadSize);
					char buf[TEST_SIZE];
					//user[i].RecvQ.Peek(buf,i+5);
					user[i].RecvQ.Dequeue(buf, i);
					
					buf[i] = '\0';

					if (strcmp(buf, user[i].buf) == 0)
					{
						printf("fucking");
					}

					//if (header.wMsgType != df_RES_STRESS_ECHO)
					//{
					//	user[i].valid = false;
					//	closesocket(user[i].sock);
					//	errorCount++;
					//	break;
					//}

					bool fail = false;

					//WORD tt;
					//p >> tt;

					//if (user[i].buf_size != tt)
					//{
					//	user[i].valid = false;
					//	closesocket(user[i].sock);
					//	errorCount++;
					//	break;
					//}


					//////////
					//for (int j = 0; j < user[i].buf_size; j++)
					//{
					//	if (user[i].buf[j] != *(p.GetBufferPtr() + j))
					//	{
					//		fail = true;
					//		break;
					//	}
					//}

					//if (fail)
					//{
					//	user[i].valid = false;
					//	closesocket(user[i].sock);
					//	//errorCount++;
					//	break;
					//}

					//p.Clear();
					user[i].recv = true;
					//allTime += timeGetTime() - user[i].echo;
					//RecvCount++;
				}

				//cnt++;
				//char buf[300];
				//size = recv(user[i].sock, buf, 300, 0);
				//
				//if (size < 0)
				//{
				//	printf("%d\n",WSAGetLastError());
				//	continue;
				//}
				//
				//buf[size] = '\0';
				//if (size == user[i].buf_size)
				//{
				//	if (strcmp(user[i].buf, buf) != 0)
				//	{
				//		system("pause");
				//	}
				//}
				//user[i].recv = true;

			}
		}
	}
	*/
	//inet_ntop(AF_INET, &pAddr, ip, sizeof(ip));
	

	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));

	if (retval == SOCKET_ERROR)
	{
		printf("error\n");
		return 0;
	}
	st_PACKET_HEADER header;

	header.byCode = 0x89;
	header.wMsgType = 100;

	while (1)
	{
		printf("send msg : ");
		scanf("%s", buf);
		int len = strlen(buf);
		header.wPayloadSize = len;
		//send(sock, (char *)&header, sizeof(header), 0);
		send(sock, buf,len, 0);

		ZeroMemory(buf, 512);
		recv(sock, buf, len, 0);

		buf[len] = '\0';

		printf("%s\n",buf);
	}
}
