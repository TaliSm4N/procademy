#pragma once
#include <windows.h>
#include <queue>

#define dfMAXCHILD		100

class GraphWindow
{
public:
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
		GraphWindow	*pThis[dfMAXCHILD];

	} stHWNDtoTHIS;

	GraphWindow(HINSTANCE hInstance, HWND hWndParent, TYPE enType, int posX, int posY, int iWidth, int iHeight);
	~GraphWindow();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	BOOL	InsertData(int iData);

protected:

	//------------------------------------------------------
	// ������ �ڵ�, this ������ ��Ī ���̺� ����.
	//------------------------------------------------------
	BOOL				PutThis(void);
	static GraphWindow	*GetThis(HWND hWnd);

private:

	//------------------------------------------------------
	// �θ� ������ �ڵ�, �� ������ �ڵ�, �ν��Ͻ� �ڵ�
	//------------------------------------------------------
	//------------------------------------------------------
	// ������ ��ġ,ũ��,����, �׷��� Ÿ�� ��.. �ڷ�
	//------------------------------------------------------
	TYPE		_enGraphType;

	//------------------------------------------------------
	// ���� ���۸��� �޸� DC, �޸� ��Ʈ��
	//------------------------------------------------------
	HDC		_hMemDC;
	HBITMAP		_hBitmap;

	//------------------------------------------------------
	// ������
	//------------------------------------------------------
	std::queue<int> DataQueue;

		// static �ɹ� �Լ��� ���ν������� This �����͸� ã�� ����
		// HWND + Class Ptr �� ���̺�
};