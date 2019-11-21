#pragma comment(lib,"ws2_32")

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <list>
#include <queue>

#include "Buffer.h"
#include "Console.h"

#define PORT 3000


#define WIDTH 80
#define HEIGHT 23

struct Player
{
	int id;
	int x;
	int y;
};

//IDÇÒ´ç(0)	Type(4Byte) | ID(4Byte) | ¾È¾¸(4Byte) | ¾È¾¸(4Byte)
//º°»ý¼º(1)	Type(4Byte) | ID(4Byte) | X(4Byte)    | Y(4Byte)
//º°»èÁ¦(2)	Type(4Byte) | ID(4Byte) | ¾È¾¸(4Byte) | ¾È¾¸(4Byte)
//ÀÌµ¿(3)	Type(4Byte) | ID(4Byte) | X(4Byte)    | Y(4Byte)
struct Message
{
	int type;
	char data[12];
};

struct SetIDMsg
{
	int type;
	int id;
	char data[8];
};

struct MakeStarMsg
{
	int type;
	int id;
	int x;
	int y;
};

struct DeleteStarMsg
{
	int type;
	int id;
	char data[8];
};

struct MoveStarMsg
{
	int type;
	int id;
	int x;
	int y;
};

struct Info
{
	SOCKET sock;
	Player *p;
};

std::list<Info *> info_list;

std::list<Player *> p_list;
//Player p_list[1000];

SOCKET sock;
FD_SET rset, wset;

std::list<SOCKET> c_sock_list;
int id = 0;


bool network();

void init();
void render();
void drawPlayer();

int main()
{
	SOCKADDR_IN addr;
	ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(PORT);

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (sock == INVALID_SOCKET)
	{
		std::cout << "socket error" << std::endl;
		system("pause");
		return -1;
	}

	if (bind(sock, (SOCKADDR *)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		std::cout << "bind error" << std::endl;
		system("pause");
		return -1;
	}

	if (listen(sock, SOMAXCONN) == SOCKET_ERROR)
	{
		std::cout << "listen error" << std::endl;
		system("pause");
		return -1;
	}

	u_long on = 1;
	if (ioctlsocket(sock, FIONBIO, &on) == SOCKET_ERROR)
	{
		std::cout << "non blocking error" << std::endl;
		system("pause");
		return -1;
	}

	
	//FD_SET err_set;
	
	

	
	while (1)
	{
		if (!network())
		{
			break;
		}
		render();

		//std::cout << "while";
	}

	system("pause");
}

bool Send_BroadCast(Message *msg,int ig)
{
	int ret;
	for (auto iter = info_list.begin(); iter != info_list.end();iter++)
	{
		if (ig != (*iter)->sock)
		{
			ret = send((*iter)->sock, (char *)msg, sizeof(Message), 0);
			//if (ret == SOCKET_ERROR)
			//{
			//	Message msg;
			//	msg.type = 2;
			//	((DeleteStarMsg *)&msg)->id = (*iter)->p->id;
			//	
			//	if (info_list.empty())
			//		break;
			//	else
			//	{
			//		iter = info_list.erase(iter);
			//		Send_BroadCast(&msg, ((DeleteStarMsg *)&msg)->id);
			//	}
			//}
			//else
			//	iter++;
		}
	}
	//std::cout << "broadCast" << std::endl;
	return true;
}

void init()
{
	cs_Initial();
	//cs_ClearScreen();
}


bool network()
{
	int addrlen, i, ret;
	Info *info;
	Message msg;

	SOCKET c_sock;
	SOCKADDR c_addr;

	FD_ZERO(&rset);
	FD_ZERO(&wset);
	//FD_ZERO(&err_set);
	FD_SET(sock, &rset);

	for (auto iter = info_list.begin(); iter != info_list.end(); iter++)
		FD_SET((*iter)->sock, &rset);


	if (ret = select(0, &rset, &wset, NULL, NULL) == SOCKET_ERROR)
	{
		std::cout << "select error" << std::endl;
		system("pause");
		return false;
	}

	//read

	if (FD_ISSET(sock, &rset))
	{
		std::cout << "sock" << std::endl;
		addrlen = sizeof(c_addr);
		c_sock = accept(sock, (SOCKADDR *)&c_addr, &addrlen);

		if (c_sock == INVALID_SOCKET)
		{
			std::cout << "c_socket error" << std::endl;
			system("pause");
			return false;
		}
		info = new Info();
		info->sock = c_sock;

		
		msg.type = 0;
		((SetIDMsg *)&msg)->id = id;
		send(c_sock, (char *)&msg, sizeof(Message), 0);

		//std::cout << "send id" << ((SetIDMsg *)&msg)->id << std::endl;

		Player *temp = new Player();
		temp->id = id++;
		temp->x = 40;
		temp->y = 11;

		info->p = temp;

	
		msg.type = 1;
		((MakeStarMsg *)&msg)->id = temp->id;
		((MakeStarMsg *)&msg)->x = temp->x;
		((MakeStarMsg *)&msg)->y = temp->y;

		Send_BroadCast(&msg, -1);
		
		info_list.push_back(info);
		for (auto iter = info_list.begin(); iter != info_list.end(); iter++)
		{
			((MakeStarMsg *)&msg)->id = (*iter)->p->id;
			((MakeStarMsg *)&msg)->x = (*iter)->p->x;
			((MakeStarMsg *)&msg)->y = (*iter)->p->y;
			send(info->sock, (char *)&msg, sizeof(Message), 0);
		}

		//std::cout << "id" << info->p->id << std::endl;




		//for (int i = 0; i < id; i++)
		//for(auto iter=p_list.begin();iter!=p_list.end();iter++)
		//{
		//	msg.type = 1;
		//	((MakeStarMsg *)&msg)->id = (*iter)->id;
		//	((MakeStarMsg *)&msg)->x = (*iter)->x;
		//	((MakeStarMsg *)&msg)->y = (*iter)->y;
		//
		//	send(c_sock, (char *)&msg, sizeof(Message), 0);
		//	std::cout << "send 1" << std::endl;
		//}
		//
		//c_sock_list.push_back(c_sock);
		//
		//Player *temp = new Player();
		//temp->id = id++;
		//temp->x = 40;
		//temp->y = 11;
		//
		//p_list.push_back(temp);
		//
		//msg.type = 1;
		//((MakeStarMsg *)&msg)->id = temp->id;
		//((MakeStarMsg *)&msg)->x = temp->x;
		//((MakeStarMsg *)&msg)->y = temp->y;
		//
		//Send_BroadCast(&msg,-1);
		//FD_SET(c_sock, &rset);
			//std::cout << "test";
		
		//std::cout << sock << std::endl;
	}
	
	for (auto iter = info_list.begin(); iter != info_list.end();)
	{
		bool die = false;
		if (FD_ISSET((*iter)->sock, &rset))
		{
			ret = 99;
			bool flag = false;

			while (ret > 0)
			{
				ret = recv((*iter)->sock, (char *)&msg, sizeof(Message), 0);

				if (ret == SOCKET_ERROR&&!flag)
				{
					die = true;
					//std::cout << "°­Á¾" << std::endl;
					continue;
				}

				switch (msg.type)
				{
				case 1:
					break;
				case 3:
					flag = true;
					//std::cout << "---"<< ((MoveStarMsg *)&msg)->id << std::endl;
					//std::cout << "x" << ((MoveStarMsg *)&msg)->x << "y" << ((MoveStarMsg *)&msg)->y << std::endl;
					break;
				default:
					//Message msg;
					//msg.type = 2;
					//((DeleteStarMsg *)&msg)->id = (*iter)->p->id;
					////iter = info_list.erase(iter);
					//Send_BroadCast(&msg, ((DeleteStarMsg *)&msg)->id);
					//die = true;
					break;
				}
			}

			if (flag)
			{
				//std::cout << "ididididi" << ((MoveStarMsg *)&msg)->id << std::endl;
				Send_BroadCast(&msg, (*iter)->sock);
				(*iter)->p->x = ((MoveStarMsg *)&msg)->x;
				(*iter)->p->y = ((MoveStarMsg *)&msg)->y;
			}
		}

		if (die)
		{
			Message msg;
			msg.type = 2;
			((DeleteStarMsg *)&msg)->id = (*iter)->p->id;
			iter = info_list.erase(iter);
			Send_BroadCast(&msg, ((DeleteStarMsg *)&msg)->id);
		}
		else
			iter++;
	}

//	//write
//
//	for (auto iter = info_list.begin(); iter != info_list.end(); iter++)
//	{
//		if (FD_ISSET((*iter)->sock, &wset))
//		{
//			while (!(*iter)->msg.empty())
//			{
//				send((*iter)->sock, (char *)(*iter)->msg.front(), sizeof(Message), 0);
//				(*iter)->msg.pop();
//				std::cout << "test";
//			}
//		}
//	}

	//for (auto iter = c_sock_list.begin(); iter != c_sock_list.end(); iter++)
	//{
	//	if (FD_ISSET(*iter, &rset))
	//	{
	//		std::cout << "read!!" << std::endl;
	//		ret = 99;
	//		while (ret > 0)
	//		{
	//			ret = recv(*iter, (char *)&msg, sizeof(Message), 0);
	//
	//
	//			switch (msg.type)
	//			{
	//			case 1:
	//				break;
	//			case 3:
	//				Send_BroadCast(&msg, *iter);
	//				std::cout << "iter" << *iter << std::endl;
	//				std::cout << "x"<< ((MoveStarMsg *)&msg)->x <<"y" << ((MoveStarMsg *)&msg)->y <<std::endl;
	//				for (auto iter = p_list.begin(); iter != p_list.end(); iter++)
	//				{
	//					if ((*iter)->id == ((MoveStarMsg *)&msg)->id)
	//					{
	//						(*iter)->x = ((MoveStarMsg *)&msg)->x;
	//						(*iter)->y == ((MoveStarMsg *)&msg)->y;
	//					}
	//				}
	//				break;
	//			default:
	//				std::cout << "different msg error" <<msg.type << std::endl;
	//				std::cout << "data:" << msg.data << std::endl;
	//				//system("pause");
	//				//return false;
	//				break;
	//			}
	//		}
	//	}
	//}

	return true;
}

void render()
{
	Buffer_Clear();
	drawPlayer();
	Buffer_Flip();
}

void drawPlayer()
{
	for (auto iter = info_list.begin(); iter != info_list.end(); iter++)
	{
		Sprite_Draw((*iter)->p->x, (*iter)->p->y, '*');
	}
}