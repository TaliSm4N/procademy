#include "MonitorGraphUnit.h"

const LPCWSTR MonitorGraphUnit::className = L"MONITOR_GRAPH_UNIT";

MonitorGraphUnit::MonitorGraphUnit(HINSTANCE hInstance, HWND hWndParent, TYPE enType, int iPosX, int iPosY, int iWidth, int iHeight,int max)
	:_hInst(hInstance),_hWndParent(hWndParent),_enGraphType(enType),_posX(iPosX),_posY(iPosY),_width(iWidth),_height(iHeight),_maxData(max)
{
	_hWnd = CreateWindow(className, L"Child", WS_CHILD | WS_VISIBLE | WS_CAPTION | WS_CLIPSIBLINGS, _posX, _posY, _width, _height, _hWndParent, 0, _hInst, NULL);
	_hPen = CreatePen(PS_SOLID, 5, RGB(255, 255, 255));//default pen;
	SetWindowLongPtr(_hWnd, 0, (LONG)this);

	switch (_enGraphType)
	{
	case MonitorGraphUnit::LINE_SINGLE:
		_dataQueue = new MQueue<int>(dfMAXDATA);
		_info = new ST_COLUMN_INFO();
		break;
	case MonitorGraphUnit::LINE_MULTI:
		break;
	case MonitorGraphUnit::NUMBER:
		break;
	case MonitorGraphUnit::ONOFF:
		break;
	case MonitorGraphUnit::PIE:
		break;
	default:
		break;
	}
}

BOOL MonitorGraphUnit::InsertData(ULONG64 u64ServerID, int iType, int iData)
{
	if (_info == NULL)
		return false;
	else
	{
		switch (_enGraphType)
		{
		case MonitorGraphUnit::LINE_SINGLE:
			if (_dataQueue->isFull())
				_dataQueue->Pop();
			_dataQueue->Push(iData);
			drawGraph(*_dataQueue);
			break;
		case MonitorGraphUnit::LINE_MULTI:
			break;
		case MonitorGraphUnit::NUMBER:
			break;
		case MonitorGraphUnit::ONOFF:
			break;
		case MonitorGraphUnit::PIE:
			break;
		default:
			break;
		}
	}
	return true;
}

BOOL MonitorGraphUnit::drawGraph(MQueue<int> &data)
{
	RECT crt;
	HDC hdc = GetDC(_hWnd);

	GetClientRect(_hWnd, &crt);
	if (_hBitmap == NULL)
	{
		_hBitmap = CreateCompatibleBitmap(hdc, crt.right, crt.bottom);
	}
	_hMemDC = CreateCompatibleDC(hdc);
	HBITMAP OldBit = (HBITMAP)SelectObject(_hMemDC, _hBitmap);

	PatBlt(_hMemDC, 0, 0, crt.right, crt.bottom, WHITENESS);
	//FillRect(_hMemDC, &crt, GetSysColorBrush(COLOR_WINDOW));
	//for (int i = 0; i < crt.right; i += 10) {
	//	MoveToEx(_hMemDC, i, 0, NULL);
	//	LineTo(_hMemDC, i, crt.bottom);
	//}
	int len = crt.right / _dataQueue->getSize();
	int loop = 0;
	int value;
	int oldX=-1;
	int oldY = -1;
	int h;
	for (int i = _dataQueue->getSize() - _dataQueue->getCnt(); i < _dataQueue->getSize(); i++)
	{
		_dataQueue->Peek(loop++, &value);
		if (value >= _maxData)
			h = 0;
		else
		{
			h = (_maxData - value) * crt.bottom / _maxData;
		}
		MoveToEx(_hMemDC, i*len,h, NULL);
		if(oldX!=-1)
			LineTo(_hMemDC, oldX, oldY);
		oldX = i * len;
		oldY = h;
	}


	SelectObject(_hMemDC, OldBit);
	DeleteObject(OldBit);
	DeleteDC(_hMemDC);
	ReleaseDC(_hWnd, hdc);
	InvalidateRect(_hWnd, NULL, FALSE);

	return true;
}

BOOL	MonitorGraphUnit::SetDataColumnInfo(int iColIndex, ULONG64 u64ServerID, int iType, WCHAR *szName)
{
	switch (_enGraphType)
	{
	case MonitorGraphUnit::LINE_SINGLE:
		_info->iType = iType;
		wcscpy_s(_info->szName, szName);
		_info->u64ServerID = u64ServerID;
		break;
	case MonitorGraphUnit::LINE_MULTI:
		break;
	case MonitorGraphUnit::NUMBER:
		break;
	case MonitorGraphUnit::ONOFF:
		break;
	case MonitorGraphUnit::PIE:
		break;
	default:
		break;
	}
	return true;
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
		wcex.cbWndExtra = 4;
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
	MonitorGraphUnit *pThis = (MonitorGraphUnit *)GetWindowLongPtr(hWnd, 0);
	HBITMAP OldBit;
	//static HBITMAP hBit;
	RECT crt;

	
	switch (message)
	{
	case WM_CREATE:
		//g = new MonitorGraphUnit(hInst, hWnd, MonitorGraphUnit::LINE_SINGLE, 10, 10, 400, 400);
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		GetClientRect(hWnd, &crt);
		pThis->_hMemDC = CreateCompatibleDC(hdc);
		OldBit = (HBITMAP)SelectObject(pThis->_hMemDC, pThis->_hBitmap);
		BitBlt(hdc, 0, 0, crt.right, crt.bottom, pThis->_hMemDC, 0, 0, SRCCOPY);
		SelectObject(pThis->_hMemDC, OldBit);
		DeleteDC(pThis->_hMemDC);

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
