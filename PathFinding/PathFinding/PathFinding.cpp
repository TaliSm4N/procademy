// PathFinding.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "PathFinding.h"
#include "ScreenDib.h"

#define MAX_LOADSTRING 100

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define BLOCK_SIZE 20

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

enum MODE {START=1,END=2,WALL=3,DELETE_WALL=4};

MODE g_mode=START;

int block[SCREEN_HEIGHT / BLOCK_SIZE][SCREEN_WIDTH / BLOCK_SIZE];

ScreenDib g_screen(SCREEN_WIDTH, SCREEN_HEIGHT, 32);

HWND g_hWnd;
void drawGrid();
void drawBlock();


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
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
			
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

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
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
	static bool Draw = false;
	static int oldX;
	static int oldY;
	int x;
	int y;
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
	case WM_LBUTTONUP:
		Draw = false;
		break;
	case WM_LBUTTONDOWN:
		Draw = true;
	case WM_MOUSEMOVE:
	{
		if (Draw)
		{
			x = LOWORD(lParam);
			y = HIWORD(lParam);

			if (x > SCREEN_WIDTH || y > SCREEN_HEIGHT)
				break;

			int *setBlock = &block[y / BLOCK_SIZE][x / BLOCK_SIZE];

			if (g_mode == DELETE_WALL)
			{
				*setBlock = 0;
			}
			else if(*setBlock==0)
			{
				if (g_mode == START || g_mode == END)
				{
					for (int i = 0; i < SCREEN_HEIGHT / BLOCK_SIZE; i++)
					{
						for (int j = 0; j < SCREEN_WIDTH / BLOCK_SIZE; j++)
						{
							if (block[i][j] == g_mode)
								block[i][j] = 0;
						}
					}
				}
				*setBlock = g_mode;

			}

			//oldX = x;
			//oldY = y;
			drawBlock();
			drawGrid();
			g_screen.Flip(hWnd);
		}
	}
		break;
	break;
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case 0x53://S key
			g_mode = START;
			break;
		case 0x45://E key
			g_mode = END;
			break;
		case 0x57://W key
			g_mode = WALL;
			break;
		case 0x44://D key
			g_mode = DELETE_WALL;
			break;
		case 0x46://F key
			break;
		}
	}
	break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
            EndPaint(hWnd, &ps);
        }
		drawBlock();
		drawGrid();
		g_screen.Flip(hWnd);
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

void drawGrid()
{
	BYTE *dib = g_screen.GetDibBuffer();
	DWORD *dest = (DWORD *)dib;
	int pitch = g_screen.GetPitch();
	for (int i = 1; i < SCREEN_HEIGHT; i++)
	{
		for (int j = 1; j < SCREEN_WIDTH; j ++)
		{
			if(i%20==0||j%20==0)
				*(dest + j + (pitch / 4)*i) = 0x00000000;
		}
	}
}

void drawBlock()
{
	BYTE *dib = g_screen.GetDibBuffer();
	DWORD *dest = (DWORD *)dib;
	int pitch = g_screen.GetPitch();
	DWORD color = 0x00000000;
	for (int i = 0; i < SCREEN_HEIGHT / BLOCK_SIZE; i++)
	{
		for (int j = 0; j < SCREEN_WIDTH / BLOCK_SIZE; j++)
		{
			switch (block[i][j])
			{
			case START:
				color = 0x00ff0000;
				break;
			case END:
				color = 0x000000ff;
				break;
			case WALL:
				color = 0x00555555;
				break;
			default:
				color = 0x00ffffff;
				//return;
				break;
			}

			for (int iDraw = i * BLOCK_SIZE; iDraw < i*BLOCK_SIZE + BLOCK_SIZE; iDraw++)
			{
				for (int jDraw = j * BLOCK_SIZE; jDraw < j*BLOCK_SIZE + BLOCK_SIZE; jDraw++)
					*(dest + jDraw + (pitch / 4)*iDraw) = color;
			}
		}
	}
}