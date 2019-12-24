// pathFinding.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "pathFinding.h"
#include "ScreenDib.h"
//#include "AStar.h"
#include "JPS.h"
#include "Map.h"
#include <windowsx.h>

#define MAX_LOADSTRING 100

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define BLOCK_SIZE 20

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.
HWND g_hWnd;
BOOL g_find = false;

//AStar *AstarPath = nullptr;

JPS *jps;

ScreenDib *g_screen;// = new ScreenDib(SCREEN_WIDTH, SCREEN_HEIGHT, 32);


// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

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
    LoadStringW(hInstance, IDC_PATHFINDING, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 응용 프로그램 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PATHFINDING));

    MSG msg;

    // 기본 메시지 루프입니다:
    //while (GetMessage(&msg, nullptr, 0, 0))
    //{
    //    if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
    //    {
    //        TranslateMessage(&msg);
    //        DispatchMessage(&msg);
    //    }
    //}

	while (1)
	{


		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			//if (g_find)
			//{
			//	if (AstarPath->Find() != 0)
			//	{
			//		g_find = false;
			//	}
			//}
	
			jps->GetMap()->draw(g_screen->GetDibBuffer(), g_screen->GetPitch(), BLOCK_SIZE);
			//AstarPath->draw(g_screen->GetDibBuffer(), g_screen->GetPitch(), BLOCK_SIZE);
			//drawGrid();
			g_screen->Flip(g_hWnd);
		}
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

    wcex.style          = CS_HREDRAW | CS_VREDRAW| CS_DBLCLKS;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PATHFINDING));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_PATHFINDING);
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

	 //HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
	 //   CW_USEDEFAULT, 0, SCREEN_WIDTH, SCREEN_HEIGHT, nullptr, nullptr, hInstance, nullptr);

	HWND hWnd = CreateWindowEx(0, szTitle, szTitle, WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, SCREEN_WIDTH, SCREEN_HEIGHT, nullptr, nullptr, hInstance, nullptr);
	g_hWnd = hWnd;
	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	SetFocus(hWnd);

	RECT WindowRect;
	WindowRect.top = 0;
	WindowRect.left = 0;
	WindowRect.right = SCREEN_WIDTH;
	WindowRect.bottom = SCREEN_HEIGHT;

	AdjustWindowRectEx(&WindowRect, GetWindowStyle(hWnd), GetMenu(hWnd) != NULL, GetWindowExStyle(hWnd));

	int x = (GetSystemMetrics(SM_CXSCREEN) / 2) - (640 / 2);
	int y = (GetSystemMetrics(SM_CYSCREEN) / 2) - (480 / 2);

	MoveWindow(hWnd, x, y, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, TRUE);

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
	static bool wall = false;
	static bool erase = false;
	


    switch (message)
    {
	case WM_CREATE:
		//AstarPath=new AStar(SCREEN_WIDTH / BLOCK_SIZE, SCREEN_HEIGHT / BLOCK_SIZE);
		jps = new JPS(SCREEN_WIDTH / BLOCK_SIZE, SCREEN_HEIGHT / BLOCK_SIZE);
		g_screen = new ScreenDib(SCREEN_WIDTH, SCREEN_HEIGHT, 32);
		break;
    //case WM_COMMAND:
    //    {
    //        int wmId = LOWORD(wParam);
    //        // 메뉴 선택을 구문 분석합니다:
    //        switch (wmId)
    //        {
    //        case IDM_ABOUT:
    //            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
    //            break;
    //        case IDM_EXIT:
    //            DestroyWindow(hWnd);
    //            break;
    //        default:
    //            return DefWindowProc(hWnd, message, wParam, lParam);
    //        }
    //    }
    //    break;
	case WM_LBUTTONDBLCLK:
	{
		g_find = false;
		//AstarPath->resetFind();
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);

		if (x > SCREEN_WIDTH || y > SCREEN_HEIGHT)
			break;
		jps->GetMap()->setTile(MODE::START, x / BLOCK_SIZE, y / BLOCK_SIZE);
		//AstarPath->setTile(START, x / BLOCK_SIZE, y / BLOCK_SIZE);
		//AstarPath->draw(g_screen->GetDibBuffer(), g_screen->GetPitch(), BLOCK_SIZE);
		g_screen->Flip(hWnd);
		wall = false;
		erase = false;
	}
	break;
	case WM_RBUTTONDBLCLK:
	{
		g_find = false;
		//AstarPath->resetFind();
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);

		if (x > SCREEN_WIDTH || y > SCREEN_HEIGHT)
			break;
		jps->GetMap()->setTile(END, x / BLOCK_SIZE, y / BLOCK_SIZE);
		//AstarPath->setTile(END, x / BLOCK_SIZE, y / BLOCK_SIZE);
		//AstarPath->draw(g_screen->GetDibBuffer(), g_screen->GetPitch(), BLOCK_SIZE);
		//g_screen->Flip(hWnd);
		wall = false;
		erase = false;
	}
	break;
	case WM_LBUTTONDOWN:
		g_find = false;
		wall = true;
		break;
	case WM_LBUTTONUP:
		g_find = false;
		wall = false;
		break;
	case WM_RBUTTONDOWN:
		g_find = false;
		erase = true;
		break;
	case WM_RBUTTONUP:
		g_find = false;
		erase = false;
		break;
	case WM_MOUSEMOVE:
		if (wall)
		{
			g_find = false;
			//AstarPath->resetFind();
			int x = LOWORD(lParam) / BLOCK_SIZE;
			int y = HIWORD(lParam) / BLOCK_SIZE;
			jps->GetMap()->setTile(MODE::WALL, x, y);
			//AstarPath->setTile(WALL, x, y);
			//AstarPath->draw(g_screen->GetDibBuffer(), g_screen->GetPitch(), BLOCK_SIZE);
			//drawGrid();
			g_screen->Flip(hWnd);
		}
		else if (erase)
		{
			g_find = false;
			//AstarPath->resetFind();
			int x = LOWORD(lParam) / BLOCK_SIZE;
			int y = HIWORD(lParam) / BLOCK_SIZE;

			if (jps->GetMap()->getTile(x, y) == WALL)
			{
				jps->GetMap()->setTile(MODE::DELETE_WALL, x, y);
			}

			//if (AstarPath->getTile(x, y) == WALL)
			//{
			//	AstarPath->setTile(DELETE_WALL, x, y);
			//}

			//AstarPath->draw(g_screen->GetDibBuffer(), g_screen->GetPitch(), BLOCK_SIZE);
			////drawGrid();
			//g_screen->Flip(hWnd);
		}
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		//Find path 
		case VK_SPACE:
			g_find = false;
			//AstarPath->Find();
			break;
		//Find path
		case 0x46://F key
			//AstarPath->resetFind();
			g_find = true;
			break;
		//clear find history
		case 0x43://C key
			g_find = false;
			//AstarPath->resetFind();
			break;
		//reset Map
		case 0x52://R key
			g_find = false;
			//AstarPath->resetMap();
			break;
		//Togle Test Mode
		case 0x54://T key
			//AstarPath->TestMode();
			break;
		//Togle Show Path Mode
		case 0x59://Y key
			//AstarPath->ShowMode();
			break;
		default:
			break;
		}
		//AstarPath->draw(g_screen->GetDibBuffer(), g_screen->GetPitch(), BLOCK_SIZE);
		////drawGrid();
		//g_screen->Flip(hWnd);
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
            EndPaint(hWnd, &ps);
			//
			//
			//AstarPath->draw(g_screen->GetDibBuffer(), g_screen->GetPitch(), BLOCK_SIZE);
			////drawGrid();
			//g_screen->Flip(hWnd);
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
