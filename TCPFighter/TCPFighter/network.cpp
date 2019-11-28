#pragma comment(lib,"ws2_32")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include "network.h"
#include "PacketDefine.h"

//#include "PacketDefine.h"

#define PORT 5000
#define BUF_SIZE 1024
#define IP "127.0.0.1"

struct Session
{
	SOCKET socket;
	RingBuffer *RecvQ;
	RingBuffer *SendQ;
};

Session g_session;
BOOL g_bSend=false;

bool networkInit(HWND hWnd,int WM_SOCKET)
{
	g_session.RecvQ = new RingBuffer(BUF_SIZE);
	g_session.SendQ = new RingBuffer(BUF_SIZE);
	int ret;
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return false;

	g_session.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	ret = WSAAsyncSelect(g_session.socket, hWnd, WM_SOCKET, FD_READ | FD_WRITE | FD_CLOSE);

	if (ret == SOCKET_ERROR)
	{
		OutputDebugString(L"AsyncSelect Error");
		return false;
	}

	SOCKADDR_IN addr;
	ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, IP, &addr.sin_addr.s_addr);
	addr.sin_port = htons(PORT);

	ret = connect(g_session.socket, (SOCKADDR *)&addr, sizeof(addr));

	if (ret == SOCKET_ERROR && GetLastError() == !WSAEWOULDBLOCK)
	{
		return false;
	}

	return true;
}

void ProcessSocketMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_CLOSE:
		//OutputDebugString(L"FD_CLOSE");
		break;
	case FD_READ:
		//system("pause");
		//OutputDebugString(L"FD_READ");
		ProcRead(hWnd);
		break;
	case FD_WRITE:
		g_bSend = true;
		ProcWrite();
		//system("pause");
		//OutputDebugString(L"FD_WRITE");
		break;
	default:
		break;
	}
}

void ProcRead(HWND hWnd)
{
	char buf[1024];
	char payLoad[100];
	int enq;
	int size;
	st_NETWORK_PACKET_HEADER header;

	size = recv(g_session.socket, buf, sizeof(buf), 0);

	if (size == SOCKET_ERROR)
	{
		//int temp = 0;
		//system("pause");
		exit(-1);
	}

	enq = g_session.RecvQ->Enqueue(buf, size);

	if (enq != size)
	{
		int temp = 0;
		//system("pause");
		exit(-1);
	}

	while (1)
	{
		if (sizeof(header) > g_session.RecvQ->GetUseSize())
			break;

		if (g_session.RecvQ->Peek((char *)&header, sizeof(header)) < sizeof(header))
			break;
		if (header.bySize + sizeof(header) >= g_session.RecvQ->GetUseSize())
			break;

		g_session.RecvQ->MoveFront(sizeof(header));

		if (g_session.RecvQ->Dequeue(payLoad, header.bySize) != header.bySize)
		{
			exit(-1);
		}

		//메시지처리
	}

	ProcWrite();
}

void ProcWrite()
{
	if (!g_bSend)
	{
		return;
	}
	char buf[1024];
	int size = g_session.SendQ->Peek(buf, 1024);

	int send_size = send(g_session.socket, buf, size, 0);

	if (send_size == SOCKET_ERROR && GetLastError() != WSAEWOULDBLOCK)
	{
		g_bSend = false;
		return;
	}

	g_session.SendQ->MoveFront(send_size);

	return;
}