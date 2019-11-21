#pragma comment(lib,"ws2_32")

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <list>

#include "Buffer.h"
#include "Console.h"

#define PORT 3000
#define FRAME 26

#define WIDTH 80
#define HEIGHT 23

struct Player
{
	int id;
	int x;
	int y;
};

//ID할당(0)	Type(4Byte) | ID(4Byte) | 안씀(4Byte) | 안씀(4Byte)
//별생성(1)	Type(4Byte) | ID(4Byte) | X(4Byte)    | Y(4Byte)
//별삭제(2)	Type(4Byte) | ID(4Byte) | 안씀(4Byte) | 안씀(4Byte)
//이동(3)	Type(4Byte) | ID(4Byte) | X(4Byte)    | Y(4Byte)
struct Message
{
	int type;
	char data[12];
};

std::list<Player> p_list;
SOCKET sock;
int g_myID;
Player *g_myP = nullptr;
void init();
void network();
void keyboard(); 
void render();
void drawPlayer();

int main()
{	

	WSADATA wsa;
	char ip[128];
	SOCKADDR_IN addr;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	std::cout << "연결할 IP를 입력하세요:";
	std::cin >> ip;

	ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &addr.sin_addr.s_addr);
	addr.sin_port = htons(PORT);

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	
	//int optval = FRAME;
	//setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&optval, sizeof(optval));

	if (sock == INVALID_SOCKET)
	{
		std::cout << "잘못된 소켓" << std::endl;
		system("pause");
		return 1;
	}
	if ((int)connect(sock, (SOCKADDR *)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		std::cout << "connect 에러" << std::endl; 
		system("pause");
		return 1;
	}

	u_long on = 1;
	ioctlsocket(sock, FIONBIO, &on);

	init();

	while (1)
	{
		keyboard();
		network();
		//렌더
		//render();
		Sleep(FRAME);
		//std::cout << "frame_check" << std::endl;
	}

	closesocket(sock);
	WSACleanup();

	return 0;

}

void init()
{
	cs_Initial();
	//cs_ClearScreen();
}

void keyboard()
{
	bool flag = false;

	if (g_myP == nullptr)
		return;

	//if (GetAsyncKeyState(VK_UP))
	if (GetAsyncKeyState(0x57))
	{
		if (g_myP->y > 0)
		{
			g_myP->y--;
			flag = true;
		}
	}

	//if (GetAsyncKeyState(VK_DOWN))
	if (GetAsyncKeyState(0x53))
	{
		if (g_myP->y < HEIGHT)
		{
			g_myP->y++;
			flag = true;
		}
	}

	//if (GetAsyncKeyState(VK_LEFT))
	if (GetAsyncKeyState(0x41))
	{
		if (g_myP->x > 0)
		{
			g_myP->x--;
			flag = true;
		}
	}

	//if (GetAsyncKeyState(VK_RIGHT))
	if (GetAsyncKeyState(0x44))
	{

		if (g_myP->x < WIDTH)
		{
			g_myP->x++;
			flag = true;
		}
	}

	if (flag)
	{
		Message msg;
		msg.type = 3;
		memcpy(msg.data, g_myP, sizeof(Player));
		send(sock, (char *)&msg, sizeof(Message), 0);
		//std::cout << "Move My player:" << g_myP->id << std::endl;
		//std::cout << "X:" << g_myP->x << " Y:" << g_myP->y << std::endl;
	}

}

void network()
{
	Message msg;
	int ret=99;
	int cnt=0;
	static int max_cnt = 0;
	static int time_cnt = 0;

	while (ret>0)
	{
		//ZeroMemory(&msg, sizeof(Message));
		ret=recv(sock, (char *)&msg, sizeof(Message), 0);
		//std::cout << "ret:" << ret << std::endl;
		if (ret <= 0)
			continue;
		cnt++;

		switch (msg.type)
		{
		case 0:
			g_myID = *((int *)msg.data);
			if (g_myP == nullptr)
			{
				for (auto iter = p_list.begin(); iter != p_list.end(); iter++)
				{
					if (*((int *)msg.data) == iter->id)
					{
						g_myP = &(*iter);
					}
				}
			}
			std::cout << "test g_myID:" << g_myID << std::endl;
			break;
		case 1:
			Player p;
			memcpy(&p, msg.data, sizeof(Player));
			p_list.push_back(p);
			//std::cout << "Make player:" << ((Player *)msg.data)->id << std::endl;

			if (g_myP == nullptr)
			{
				for (auto iter = p_list.begin(); iter != p_list.end(); iter++)
				{
					if (*((int *)msg.data) == iter->id)
					{
						g_myP = &(*iter);
					}
				}
			}

			break;
		case 2:
			for (auto iter = p_list.begin(); iter != p_list.end(); iter++)
			{
				if (*((int *)msg.data) == iter->id)
				{
					p_list.erase(iter);
					break;
				}
			}
			//std::cout << "Delete player:" << ((Player *)msg.data)->id << std::endl;
			break;
		case 3:
			for (auto iter = p_list.begin(); iter != p_list.end(); iter++)
			{
				if (((Player *)msg.data)->id == iter->id)
				{
					iter->x = ((Player *)msg.data)->x;
					iter->y = ((Player *)msg.data)->y;
				}
			}
			//std::cout << "Move player:" << ((Player *)msg.data)->id << std::endl;
			//std::cout << "X:" << ((Player *)msg.data)->x << " Y:" << ((Player *)msg.data)->y << std::endl;
			break;
		default:
			//std::cout << msg.data << std::endl;
			break;
		}
	}
	

	if (time_cnt >= 15)
	{
		time_cnt = 0;
		max_cnt = 0;
	}

	if (cnt > max_cnt)
		max_cnt = cnt;
	//std::cout << "cnt:" << cnt << std::endl;
	//std::cout << "max_cnt:" << max_cnt << std::endl;
	//system("cls");
	time_cnt++;
}

void render()
{
	Buffer_Clear();
	drawPlayer();
	Buffer_Flip();
}

void drawPlayer()
{
	for (auto iter = p_list.begin(); iter != p_list.end(); iter++)
	{
		Sprite_Draw(iter->x, iter->y, '*');
	}
}