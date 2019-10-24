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

		// 여기에 Queue 를 추가하여도 됨.

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
	// 윈도우 프로시저
	/////////////////////////////////////////////////////////
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	/////////////////////////////////////////////////////////
	// 데이터 넣기.
	/////////////////////////////////////////////////////////
	BOOL	InsertData(ULONG64 u64ServerID, int iType, int iData);
	BOOL	SetDataColumnInfo(int iColIndex, ULONG64 u64ServerID, int iType, WCHAR *szName);

//protected:
//
//	//------------------------------------------------------
//	// 윈도우 핸들, this 포인터 매칭 테이블 관리.
//	//------------------------------------------------------
//	BOOL				PutThis(void);
//	static MonitorGraphUnit *GetThis(HWND hWnd);

private:
	BOOL drawGraph(MQueue<int> &data);
	//------------------------------------------------------
	// 부모 윈도우 핸들, 내 윈도우 핸들, 인스턴스 핸들
	//------------------------------------------------------
	HINSTANCE _hInst;
	HWND _hWnd;
	HWND _hWndParent;
	//------------------------------------------------------
	// 윈도우 위치,크기,색상, 그래프 타입 등.. 자료
	//------------------------------------------------------
	TYPE		_enGraphType;
	int _posX;
	int _posY;
	int _width;
	int _height;
	COLOR16 _color;

	int _maxData;//데이터의 최대값

	//------------------------------------------------------
	// 더블 버퍼링용 메모리 DC, 메모리 비트맵
	//------------------------------------------------------
	HDC			_hMemDC;
	HBITMAP		_hBitmap;

	//------------------------------------------------------
	// 펜과 같은 그리기용 데이터
	//------------------------------------------------------
	HPEN _hPen;

	//------------------------------------------------------
	// 데이터
	//------------------------------------------------------
		// static 맴버 함수의 프로시저에서 This 포인터를 찾기 위한
		// HWND + Class Ptr 의 테이블
	MQueue<int> *_dataQueue;
	stColumnInfo *_info;
};
