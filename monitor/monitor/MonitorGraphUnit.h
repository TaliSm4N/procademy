#pragma once
#include <Windows.h>
#include <windowsx.h>
#include "resource.h"
#include "MyQueue.h"

#define dfMAXCHILD		100
#define dfMAXDATA		100

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

	typedef struct ST_COLUMN_INFO
	{
		ULONG64 u64ServerID;
		int iType;//1
		WCHAR szName[64];

		// ���⿡ Queue �� �߰��Ͽ��� ��.

	} stColumnInfo;

	//typedef struct ST_HWNDtoTHIS
	//{
	//	HWND			hWnd[dfMAXCHILD];
	//	MonitorGraphUnit	*pThis[dfMAXCHILD];
	//
	//} stHWNDtoTHIS;

public:

	MonitorGraphUnit(HINSTANCE hInstance, HWND hWndParent, TYPE enType, int iPosX, int iPosY, int iWidth, int iHeight,int max);
	~MonitorGraphUnit();

	static ATOM Register(HINSTANCE hInstance);

	/////////////////////////////////////////////////////////
	// ������ ���ν���
	/////////////////////////////////////////////////////////
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	/////////////////////////////////////////////////////////
	// ������ �ֱ�.
	/////////////////////////////////////////////////////////
	BOOL	InsertData(ULONG64 u64ServerID, int iType, int iData);
	BOOL	SetDataColumnInfo(int iColIndex, ULONG64 u64ServerID, int iType, WCHAR *szName);

//protected:
//
//	//------------------------------------------------------
//	// ������ �ڵ�, this ������ ��Ī ���̺� ����.
//	//------------------------------------------------------
//	BOOL				PutThis(void);
//	static MonitorGraphUnit *GetThis(HWND hWnd);

private:
	BOOL drawGraph(MQueue<int> &data);
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
	COLOR16 _color;

	int _maxData;//�������� �ִ밪

	//------------------------------------------------------
	// ���� ���۸��� �޸� DC, �޸� ��Ʈ��
	//------------------------------------------------------
	HDC			_hMemDC;
	HBITMAP		_hBitmap;

	//------------------------------------------------------
	// ��� ���� �׸���� ������
	//------------------------------------------------------
	HPEN _hPen;

	//------------------------------------------------------
	// ������
	//------------------------------------------------------
		// static �ɹ� �Լ��� ���ν������� This �����͸� ã�� ����
		// HWND + Class Ptr �� ���̺�
	MQueue<int> *_dataQueue;
	stColumnInfo *_info;
};
