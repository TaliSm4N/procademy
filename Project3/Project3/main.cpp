#pragma comment(lib,"ws2_32")
#pragma comment(lib, "winmm.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include "Protocol.h"
#include "Packet.h"
#include "RingBuffer.h"

#include <ctime>

#define TEST_SIZE 1500

struct User
{
	SOCKET sock;
	char buf[81 * 3 + 1];
	int buf_size;
	bool recv;
	RingBuffer RecvQ;
	bool valid;
	int echo;
};

//SOCKET sock[TEST_SIZE];
char testStr[81 * 3 + 1] = "1234567890 abcdefghijklmnopqrstuvwxyz 1234567890 abcdefghijklmnopqrstuvwxyz 123451234567890 abcdefghijklmnopqrstuvwxyz 1234567890 abcdefghijklmnopqrstuvwxyz 123451234567890 abcdefghijklmnopqrstuvwxyz 1234567890 abcdefghijklmnopqrstuvwxyz 12345";

User user[TEST_SIZE];
Packet p;

int connectTry = 0;
int connectFail = 0;
int connectSuccess = 0;
int RecvCount = 0;
int errorCount = 0;


int main()
{
	WSADATA wsa;
	char ip[128];
	SOCKADDR_IN addr;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;
	st_PACKET_HEADER header;
	

	std::cout << "연결할 IP를 입력하세요:";
	std::cin >> ip;

	ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &addr.sin_addr.s_addr);
	addr.sin_port = htons(dfNETWORK_PORT);

	srand(time(NULL));
	timeBeginPeriod(1);

	for (int i = 0; i < TEST_SIZE; i++)
	{
		user[i].sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		connectTry++;
		if (user[i].sock == INVALID_SOCKET)
		{
			connectFail++;
			user[i].valid = false;
			continue;
			//std::cout << "잘못된 소켓" << std::endl;
			//system("pause");
			//return 1;
		}

		if ((int)connect(user[i].sock, (SOCKADDR *)&addr, sizeof(addr)) == SOCKET_ERROR)
		{
			connectFail++;
			user[i].valid = false;
			continue;
			//std::cout << "connect 에러" << std::endl;
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
		connectSuccess++;
	}
	WORD temp;
	DWORD tick=timeGetTime();
	int allTime = 0;
	int avr=0;
	
	while (1)
	{
		if (tick + 1000 < timeGetTime())
		{
			tick = timeGetTime();
			if (RecvCount != 0)
			{
				avr = allTime / RecvCount;
				std::cout << "////////////////////////////" << std::endl;
				std::cout << "connectTry:" << connectTry << std::endl;
				std::cout << "connectFail:" << connectFail << std::endl;
				std::cout << "connectSuccess:" << connectSuccess << std::endl;
				std::cout << "echo avr Laytency:" << avr << "ms / Recv Count:" << RecvCount << " / Error Count:" << errorCount << std::endl;
				std::cout << std::endl;
			}
			else
			{
				std::cout << "////////////////////////////" << std::endl;
				std::cout << "connectTry:" << connectTry << std::endl;
				std::cout << "connectFail:" << connectFail << std::endl;
				std::cout << "connectSuccess:" << connectSuccess << std::endl;
			}
			avr = 0;
			RecvCount = 0;
			allTime = 0;
		}

		for (int i = 0; i < TEST_SIZE; i++)
		{
			if (!user[i].valid)
				continue;

			int size;
			if (user[i].recv)
			{
				temp = rand() % (81 * 3 + 1);
				memcpy(user[i].buf, testStr, temp);
				header.byCode = dfPACKET_CODE;
				header.wMsgType = df_REQ_STRESS_ECHO;
				

				send(user[i].sock, (char *)&header, sizeof(header), 0);

				p << temp;
				p.PutData(user[i].buf, temp);
				header.wPayloadSize = p.GetBufferSize();
				send(user[i].sock, p.GetBufferPtr(), p.GetBufferSize(),0);

				p.Clear();
				user[i].recv = false;
				user[i].buf_size = temp;
				user[i].echo = timeGetTime();
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
					if (sizeof(header) > user[i].RecvQ.GetUseSize())
						break;
					if (user[i].RecvQ.Peek((char*)&header, sizeof(header)) < sizeof(header))
						break;

					if (header.byCode != dfPACKET_CODE)
					{
						user[i].valid = false;
						closesocket(user[i].sock);
						errorCount++;
						break;
					}

					if (user[i].RecvQ.GetUseSize() < header.wPayloadSize + sizeof(header))
						break;

					user[i].RecvQ.MoveFront(sizeof(header));
					user[i].RecvQ.Dequeue(p, header.wPayloadSize);

					
					if (header.wMsgType != df_RES_STRESS_ECHO)
					{
						user[i].valid = false;
						closesocket(user[i].sock);
						errorCount++;
						break;
					}

					bool fail = false;
					
					WORD tt;
					p >> tt;

					if(user[i].buf_size!=tt)
					{
						user[i].valid = false;
						closesocket(user[i].sock);
						errorCount++;
						break;
					}
					
					for (int j = 0; j < user[i].buf_size; j++)
					{
						if (user[i].buf[j] != *(p.GetBufferPtr() + j))
						{
							fail = true;
							break;
						}
					}
					
					if (fail)
					{
						user[i].valid = false;
						closesocket(user[i].sock);
						errorCount++;
						break;
					}

					p.Clear();
					user[i].recv = true;
					allTime += timeGetTime() - user[i].echo;
					RecvCount++;
				}

			}
		}
	}

	return 0;
}