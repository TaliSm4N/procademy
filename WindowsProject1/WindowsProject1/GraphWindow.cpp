#include "GraphWindow.h"

const LPCWSTR GraphWindow::className=L"GraphWindow";

GraphWindow::GraphWindow(HINSTANCE hInstance, HWND hWndParent, TYPE enType, int posX, int posY, int iWidth, int iHeight)
	:_hInstance(hInstance),_hWndParent(hWndParent),_enGraphType(enType),_iWindowPosX(posX),_iWindowPosY(posY),_iWindowPosWidth(iWidth),_iWindowPosHeight(iHeight)
{
	_hWnd = CreateWindow(className, L"Child", WS_CHILD | WS_VISIBLE | WS_CAPTION | WS_CLIPSIBLINGS, _iWindowPosX, _iWindowPosY, _iWindowPosWidth, _iWindowPosHeight, _hWndParent, 0, _hInstance, NULL);
	//LONG p =GetWindowLongPtr(_hWnd, 0);
	//SetWindowLongPtr(_hWnd, 0, (LONG)this);
	//p = GetWindowLongPtr(_hWnd, 0);
	//
	//switch (enType)
	//{
	//case LINE_SINGLE:
	//	DataQueue = new std::queue<int>();
	//	DataQueue->push(0);
	//	break;
	//case LINE_MULTI:
	//	break;
	//case NUMBER:
	//	break;
	//case ONOFF:
	//	break;
	//case PIE:
	//	break;
	//default:
	//	break;
	//}
	//
	//tagRECT Rect;
	//GetClientRect(_hWnd, &Rect);
	//HDC hdc = GetDC(_hWnd);
	//_hMemDC = CreateCompatibleDC(hdc);
	//_hBitmap = CreateCompatibleBitmap(hdc, Rect.right, Rect.bottom);
	//ReleaseDC(_hWnd, hdc);
	//
	//PatBlt(_hMemDC, 0, 0, Rect.right, Rect.bottom, WHITENESS);

}

BOOL GraphWindow::InsertData(ULONG64 u64ServerID, int iType, int iData)
{
	switch (_enGraphType)
	{
	case LINE_SINGLE:
		DataQueue->push(iData);

		while (DataQueue->size() > MAXDATA)
		{
			DataQueue->pop();
		}
		break;
	case LINE_MULTI:
		break;
	case NUMBER:
		break;
	case ONOFF:
		break;
	case PIE:
		break;
	default:
		break;
	}
	


	return true;
}

LRESULT CALLBACK GraphWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	GraphWindow *pThis = (GraphWindow *)GetWindowLongPtr(hWnd, 0);
	switch (message)
	{
	case WM_CREATE:
		SetTimer(hWnd, 1, 10,NULL);
		break;
	case WM_TIMER:
		switch (wParam)
		{
		case 1:
			OutputDebugString(L"test");
			pThis->DataQueue->push(rand()%500);
			break;
		}
		break;
	case WM_PAINT:
	{
		
		pThis->paint(hWnd);
	}
	break;
	case WM_MOUSEMOVE:
		if (wParam&MK_LBUTTON)
		{
			static int iOldPosX;
			static int iOldPosY;
			static int penSize;
			static int penColor;
			static HPEN hPen;
			static HDC hMemDC;
			static BITMAP hMemBitmap;
			int xPos = GET_X_LPARAM(lParam);
			int yPos = GET_Y_LPARAM(lParam);
			HDC hdc = GetDC(hWnd);
			HPEN hOldPen = SelectPen(hdc, hPen);
			MoveToEx(hdc, iOldPosX, iOldPosY, NULL);
			LineTo(hdc, xPos, yPos);
			//SetPixel(hdc, xPos, yPos, RGB(255, 0, 0));
			//SelectPen(hdc,hOldPen);
			ReleaseDC(hWnd, hdc);
			iOldPosX = xPos;
			iOldPosY = yPos;
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

BOOL GraphWindow::SetDataColumnInfo(int iColIndex, ULONG64 u64ServerID, int iType, LPCWSTR szName)
{
	_dataColumn.u64ServerID = u64ServerID;
	_dataColumn.iType = iType;
	wcscpy_s(_dataColumn.szName, szName);

	return true;
}

BOOL GraphWindow::paint(HWND hWnd)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);
	// TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
	
	switch (_enGraphType)
	{
	case LINE_SINGLE:
		//static int iOldPosX;
		//static int iOldPosY;
		//static int penSize;
		//static int penColor;
		//static HPEN hPen;
		//static HDC hMemDC;
		//static BITMAP hMemBitmap;
		//int xPos = GET_X_LPARAM(lParam);
		//int yPos = GET_Y_LPARAM(lParam);
		//HDC hdc = GetDC(hWnd);
		//HPEN hOldPen = SelectPen(hdc, hPen);
		//MoveToEx(hdc, iOldPosX, iOldPosY, NULL);
		//LineTo(hdc, xPos, yPos);
		////SetPixel(hdc, xPos, yPos, RGB(255, 0, 0));
		////SelectPen(hdc,hOldPen);
		//ReleaseDC(hWnd, hdc);
		//iOldPosX = xPos;
		//iOldPosY = yPos;
	{
		int len = _iWindowPosWidth / MAXDATA;
		int oldPosX = len * (MAXDATA - DataQueue->size());
		int oldPosY = DataQueue->front();
		int posX;
		int posY;
		DataQueue->push(oldPosY);
		DataQueue->pop();
		HDC hdc = GetDC(hWnd);
		for (int i = MAXDATA - DataQueue->size() + 1; i < MAXDATA; i++)
		{
			MoveToEx(hdc, oldPosX, oldPosY, NULL);
			posX = len * i;
			posY = DataQueue->front();
			LineTo(hdc, posX, posY);
			oldPosX = posX;
			oldPosY = posY;
		}
		ReleaseDC(hWnd, hdc);
	}
		break;
	case LINE_MULTI:
		break;
	case NUMBER:
		break;
	case ONOFF:
		break;
	case PIE:
		break;
	default:
		break;
	}
	EndPaint(hWnd, &ps);

	return true;
}