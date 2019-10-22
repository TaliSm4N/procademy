#pragma once
#include <Windows.h>
#include <windowsx.h>
#include "resource.h"

#define dfMAXCHILD		100

class MonitorGraphUnit
{
public:
	static const LPCWSTR className;// = L"MONITOR_GRAPH_UNIT";
	enum TYPE
	{
		LINE_SINGLE,
		LINE_MULTI,
		NUMBER,
		ONOFF,
		PIE
	};

	typedef struct ST_HWNDtoTHIS
	{
		HWND			hWnd[dfMAXCHILD];
		MonitorGraphUnit	*pThis[dfMAXCHILD];

	} stHWNDtoTHIS;

public:

	MonitorGraphUnit(HINSTANCE hInstance, HWND hWndParent, TYPE enType, int iPosX, int iPosY, int iWidth, int iHeight);
	~MonitorGraphUnit();

	static ATOM Register(HINSTANCE hInstance);

	/////////////////////////////////////////////////////////
	// ������ ���ν���
	/////////////////////////////////////////////////////////
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	/////////////////////////////////////////////////////////
	// ������ �ֱ�.
	/////////////////////////////////////////////////////////
	BOOL	InsertData(int iData);

protected:

	//------------------------------------------------------
	// ������ �ڵ�, this ������ ��Ī ���̺� ����.
	//------------------------------------------------------
	BOOL				PutThis(void);
	static MonitorGraphUnit *GetThis(HWND hWnd);

private:

	//------------------------------------------------------
	// �θ� ������ �ڵ�, �� ������ �ڵ�, �ν��Ͻ� �ڵ�
	//------------------------------------------------------
	HINSTANCE _hInst;
	HWND _hWnd;
	HWND _hWndParent;
	//------------------------------------------------------
	// ������ ��ġ,ũ��,����, �׷��� Ÿ�� ��.. �ڷ�
	//------------------------------------------------------
	TYPE		_enGraphType;
	int _posX;
	int _posY;
	int _width;
	int _height;

	//------------------------------------------------------
	// ���� ���۸��� �޸� DC, �޸� ��Ʈ��
	//------------------------------------------------------
	HDC		_hMemDC;
	HBITMAP		_hBitmap;




	//------------------------------------------------------
	// ������
	//------------------------------------------------------
		// static �ɹ� �Լ��� ���ν������� This �����͸� ã�� ����
		// HWND + Class Ptr �� ���̺�
};
