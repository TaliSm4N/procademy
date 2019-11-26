// DrawPenClient.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//


#include "stdafx.h"
#include "DrawPenClient.h"
#include <iostream>
#define BUF_SIZE 1024
#define PORT 25000

#define WM_SOCKET (WM_USER+1)

struct st_DRAW_PACKET
{
	int iStartX;
	int iStartY;
	int iEndX;
	int iEndY;
};

struct st_MESSAGE
{
	short header;
	char packet[sizeof(st_DRAW_PACKET)];
};

struct Session
{
	SOCKET socket;
	RingBuffer *RecvQ;
	RingBuffer *SendQ;
};

Session g_session;
bool g_bSend=false;

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void ProcessSocketMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void ProcRead(HWND hWnd);
void ProcWrite();
void PacketDraw(HWND hWnd,st_DRAW_PACKET *draw);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DRAWPENCLIENT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 응용 프로그램 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DRAWPENCLIENT));

    MSG msg;

    // 기본 메시지 루프입니다:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

		if (g_session.SendQ->GetUseSize() > 0)
		{
			ProcWrite();
		}

		//if (g_session.SendQ->GetUseSize() > 0)
		//{
		//	char buf[100];
		//	g_session.SendQ->Dequeue(buf, 100);
		//	send(g_session.socket, buf, 100, 0);
		//}
    }

    return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DRAWPENCLIENT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName = NULL;//MAKEINTRESOURCEW(IDC_DRAWPENCLIENT);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 640, 480, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 응용 프로그램 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int ret;
	static bool Draw = false;
	static int oldX;
	static int oldY;
	int x;
	int y;
	st_MESSAGE msg;

    switch (message)
    {
	case WM_CREATE:
		g_session.RecvQ = new RingBuffer(BUF_SIZE);
		g_session.SendQ = new RingBuffer(BUF_SIZE);
		WSADATA wsa;
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
			return 1;
		
		g_session.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		ret = WSAAsyncSelect(g_session.socket, hWnd, WM_SOCKET, FD_READ|FD_WRITE|FD_CLOSE);

		if (ret == SOCKET_ERROR)
		{
			OutputDebugString(L"AsyncSelect Error");
			exit(-1);
		}

		SOCKADDR_IN addr;
		ZeroMemory(&addr, sizeof(addr));
		addr.sin_family = AF_INET;
		inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);
		addr.sin_port = htons(PORT);

		ret = connect(g_session.socket, (SOCKADDR *)&addr, sizeof(addr));

		if (ret == SOCKET_ERROR&&GetLastError()==!WSAEWOULDBLOCK)
		{
			OutputDebugString(L"Connect Error");
			system("pause");
			exit(-1);
		}
		OutputDebugString(_T("Create"));
		break;
	case WM_SOCKET:
		ProcessSocketMessage(hWnd, message, wParam, lParam);
		break;
	case WM_LBUTTONDOWN :
		Draw = true;
		oldX = LOWORD(lParam);
		oldY = HIWORD(lParam);
		break;
	case WM_LBUTTONUP:
		Draw = false;
		break;
	case WM_MOUSEMOVE:
		if (Draw)
		{
			x = LOWORD(lParam);
			y = HIWORD(lParam);
			msg.header = 16;
			((st_DRAW_PACKET *)msg.packet)->iStartX = oldX;
			((st_DRAW_PACKET *)msg.packet)->iStartY = oldY;
			((st_DRAW_PACKET *)msg.packet)->iEndX = x;
			((st_DRAW_PACKET *)msg.packet)->iEndY = y;
			oldX = x;
			oldY = y;

			std::cout << sizeof(msg);
			if(g_session.SendQ->GetFreeSize()>sizeof(msg))
				g_session.SendQ->Enqueue((char *)&msg, sizeof(msg));
			//send(g_session.socket, (char *)&msg, 18, 0);
			//HDC hdc = GetDC(hWnd);
			//MoveToEx(hdc, oldX, oldY, NULL);
			//oldX = LOWORD(lParam);
			//oldY = HIWORD(lParam);
			//LineTo(hdc, oldX, oldY);
			//ReleaseDC(hWnd, hdc);
		}
    case WM_COMMAND:
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void ProcessSocketMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_CLOSE:
		OutputDebugString(L"FD_CLOSE");
		break;
	case FD_READ:
		//system("pause");
		OutputDebugString(L"FD_READ");
		ProcRead(hWnd);
		break;
	case FD_WRITE:
		g_bSend = true;
		ProcWrite();
		//system("pause");
		OutputDebugString(L"FD_WRITE");
		break;
	default:
		break;
	}
}

void ProcRead(HWND hWnd)
{
	char buf[1024];
	int enq;
	int size;

	st_MESSAGE *msg;

	size = recv(g_session.socket, buf, sizeof(buf),0);

	if (size == SOCKET_ERROR)
	{
		int temp = 0;
		//system("pause");
		//exit(-1);
	}

	enq = g_session.RecvQ->Enqueue(buf, size);

	if (enq != size)
	{
		int temp = 0;
		//system("pause");
		//exit(-1);
	}

	while (1)
	{
		msg = new st_MESSAGE();
		if (g_session.RecvQ->GetUseSize() < 2)
			break;
		if (g_session.RecvQ->Peek((char *)&(msg->header), sizeof(msg->header)) < 2)
			break;
		if (msg->header + 2 >= g_session.RecvQ->GetUseSize())
			break;
		g_session.RecvQ->MoveFront(2);
		if (g_session.RecvQ->Dequeue(msg->packet, msg->header) != msg->header)
		{
			int temp = 0;
			//system("pause");
			//exit(-1);
		}
		PacketDraw(hWnd, (st_DRAW_PACKET *)&(msg->packet));

		delete msg;
	}

	ProcWrite();
}

void PacketDraw(HWND hWnd, st_DRAW_PACKET *draw)
{
	//HDC hdc = GetDC(hWnd);
	//MoveToEx(hdc, oldX, oldY, NULL);
	//oldX = LOWORD(lParam);
	//oldY = HIWORD(lParam);
	//LineTo(hdc, oldX, oldY);
	//ReleaseDC(hWnd, hdc);

	HDC hdc = GetDC(hWnd);
	MoveToEx(hdc, draw->iStartX, draw->iStartY, NULL);
	LineTo(hdc, draw->iEndX, draw->iEndY);
	ReleaseDC(hWnd, hdc);
}


void ProcWrite()
{
	if (!g_bSend)
	{
		return;
	}
	char buf[1024];
	int size = g_session.SendQ->Peek(buf,1024);

	int send_size = send(g_session.socket, buf, size, 0);

	if (send_size == SOCKET_ERROR && GetLastError() != WSAEWOULDBLOCK)
	{
		g_bSend = false;
		return;
	}

	g_session.SendQ->MoveFront(send_size);

	return;
}