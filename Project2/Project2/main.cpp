#pragma comment(lib,"ws2_32")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>

#include "RingBuffer.h"

#define BUF_SIZE 50
#define PORT 25000

#define WM_SOCKET (WM_USER+1)

struct st_DRAW_PACKET
{
	int iStartX;
	int iStartY;
	int iEndX;
	int iEndY;
};

struct Session
{
	SOCKET socket;
	RingBuffer *RecvQ;
	RingBuffer *SendQ;
};

Session g_session;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;//MAKEINTRESOURCEW(IDC_DRAWPENCLIENT);
	wcex.lpszClassName = L"DrawClient";
	
	if (!RegisterClassExW(&wcex)) return 1;

	HWND hWnd = CreateWindowW(L"DrawClient", L"DrawClient", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 640, 480, nullptr, nullptr, hInstance, nullptr);
	if (hWnd == nullptr) return 1;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM WParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		g_session.RecvQ = new RingBuffer(BUF_SIZE);
		g_session.SendQ = new RingBuffer(BUF_SIZE);

		WSADATA wsa;
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
			return 1;
		g_session.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (WSAAsyncSelect(g_session.socket, hWnd, WM_SOCKET, FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR)
		{
			return -1;
		}
	}
}