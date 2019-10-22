#include "MonitorGraphUnit.h"

const LPCWSTR MonitorGraphUnit::className = L"MONITOR_GRAPH_UNIT";

MonitorGraphUnit::MonitorGraphUnit(HINSTANCE hInstance, HWND hWndParent, TYPE enType, int iPosX, int iPosY, int iWidth, int iHeight)
	:_hInst(hInstance),_hWndParent(hWndParent),_enGraphType(enType),_posX(iPosX),_posY(iPosY),_width(iWidth),_height(iHeight)
{
	_hWnd = CreateWindow(className, L"Child", WS_CHILD | WS_VISIBLE | WS_CAPTION | WS_CLIPSIBLINGS, _posX, _posY, _width, _height, _hWndParent, 0, _hInst, NULL);
}

ATOM MonitorGraphUnit::Register(HINSTANCE hInstance)
{
	static BOOL flag=true;
	if (flag)
	{
		WNDCLASSEXW wcex;

		wcex.cbSize = sizeof(WNDCLASSEX);

		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MONITOR));
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = NULL;//MAKEINTRESOURCEW(IDC_MONITOR);
		wcex.lpszClassName = className;
		wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

		flag = false;

		return RegisterClassEx(&wcex);
	}
	else
		return 0;
}
LRESULT CALLBACK MonitorGraphUnit::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//static MonitorGraphUnit *g;
	switch (message)
	{
	case WM_CREATE:
		//g = new MonitorGraphUnit(hInst, hWnd, MonitorGraphUnit::LINE_SINGLE, 10, 10, 400, 400);
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
