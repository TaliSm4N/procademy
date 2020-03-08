#include <windows.h>
#include <Pdh.h>
#include <iostream>
#pragma comment(lib,"Pdh.lib")

#include "CpuUsage.h"
//----------------------------------------------------------------------
// ������, Ȯ�δ�� ���μ��� �ڵ�. ���Է½� �ڱ� �ڽ�.
//----------------------------------------------------------------------
CCpuUsage::CCpuUsage(HANDLE hProcess)
{
	//------------------------------------------------------------------
	// ���μ��� �ڵ� �Է��� ���ٸ� �ڱ� �ڽ��� �������...
	//------------------------------------------------------------------
	if (hProcess == INVALID_HANDLE_VALUE)
	{
		_hProcess = GetCurrentProcess();
	}

	WCHAR processName[MAX_PATH];
	DWORD nameSize = MAX_PATH;
	WCHAR *tok;
	WCHAR *now=NULL;
	WCHAR *prev=NULL;

	//���μ��� �̸� ���ϱ�
	//QueryFullProcessImageName(_hProcess, 0, processName, &nameSize);
	//
	//tok = processName;
	//now = wcstok_s(tok, L"\\", &tok);
	//while (now != NULL)
	//{
	//	prev = now;
	//	now = wcstok_s(tok, L"\\", &tok);
	//}
	//tok=wcstok_s(prev, L".exe", &prev);
	//wcscpy_s(_processName, tok);
	//wprintf(L"%s\n", _processName);
	
	Init();
}

void CCpuUsage::Init()
{
	//------------------------------------------------------------------
	// ���μ��� ������ Ȯ���Ѵ�.
	//
	// ���μ��� (exe) ����� ���� cpu ������ �����⸦ �Ͽ� ���� ������ ����.
	//------------------------------------------------------------------
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);
	_iNumberOfProcessors = SystemInfo.dwNumberOfProcessors;
	_fProcessorTotal = 0;
	_fProcessorUser = 0;
	_fProcessorKernel = 0;
	_fProcessTotal = 0;
	_fProcessUser = 0;
	_fProcessKernel = 0;
	_ftProcessor_LastKernel.QuadPart = 0;
	_ftProcessor_LastUser.QuadPart = 0;
	_ftProcessor_LastIdle.QuadPart = 0;
	_ftProcess_LastUser.QuadPart = 0;
	_ftProcess_LastKernel.QuadPart = 0;
	_ftProcess_LastTime.QuadPart = 0;

	//PDH����

	WCHAR queryStr[500];

	PdhOpenQuery(NULL, NULL, &_query);
	PdhAddCounter(_query, L"\\Process(_Total)\\Private Bytes", NULL, &_privateTotalCounter);

	wsprintf(queryStr, L"\\Process(%s)\\Private Bytes", _processName);
	wprintf(L"%s\n", queryStr);
	PdhAddCounter(_query, queryStr, NULL, &_privateCounter);

	PdhAddCounter(_query, L"\\Process(_Total)\\Pool Nonpaged Bytes", NULL, &_processNonPagedTotalCounter);

	wsprintf(queryStr, L"\\Process(%s)\\Pool Nonpaged Bytes", _processName);
	wprintf(L"%s\n", queryStr);
	PdhAddCounter(_query, queryStr, NULL, &_processNonPagedCounter);

	PdhAddCounter(_query, L"\\Memory\\Available MBytes", NULL, &_availableMByteCounter);
	PdhAddCounter(_query, L"\\Memory\\Pool Nonpaged Bytes", NULL, &_poolNonpagedByteCounter);

	_privateBytes = 0;
	_privateBytesTotal = 0;

	Update();
}


void CCpuUsage::Update()
{
	PDH_FMT_COUNTERVALUE counterVal;

	UpdateCpuTime();

	PdhCollectQueryData(_query);

	PdhGetFormattedCounterValue(_privateTotalCounter, PDH_FMT_LARGE, NULL, &counterVal);
	_privateBytesTotal = counterVal.largeValue;

	PdhGetFormattedCounterValue(_privateCounter, PDH_FMT_LARGE, NULL, &counterVal);
	_privateBytes = counterVal.largeValue;

	PdhGetFormattedCounterValue(_processNonPagedTotalCounter, PDH_FMT_LARGE, NULL, &counterVal);
	_processNonPagedBytesTotal = counterVal.largeValue;

	PdhGetFormattedCounterValue(_processNonPagedCounter, PDH_FMT_LARGE, NULL, &counterVal);
	_processNonPagedBytes = counterVal.largeValue;

	PdhGetFormattedCounterValue(_availableMByteCounter, PDH_FMT_LARGE, NULL, &counterVal);
	_availableMBytes = counterVal.largeValue;

	PdhGetFormattedCounterValue(_poolNonpagedByteCounter, PDH_FMT_LARGE, NULL, &counterVal);
	_poolNonpagedBytes = counterVal.largeValue;
}

////////////////////////////////////////////////////////////////////////
// CPU ������ �����Ѵ�. 500ms ~ 1000ms ������ ȣ���� �����ѵ�.
//
//
////////////////////////////////////////////////////////////////////////
void CCpuUsage::UpdateCpuTime()
{
	//---------------------------------------------------------
	// ���μ��� ������ �����Ѵ�.
	//
	// ������ ��� ����ü�� FILETIME ������, ULARGE_INTEGER �� ������ �����Ƿ� �̸� �����.
	// FILETIME ����ü�� 100 ���뼼���� ������ �ð� ������ ǥ���ϴ� ����ü��.
	//---------------------------------------------------------
	ULARGE_INTEGER Idle;
	ULARGE_INTEGER Kernel;
	ULARGE_INTEGER User;
	//---------------------------------------------------------
	// �ý��� ��� �ð��� ���Ѵ�.
	//
	// ���̵� Ÿ�� / Ŀ�� ��� Ÿ�� (���̵�����) / ���� ��� Ÿ��
	//---------------------------------------------------------
	if (GetSystemTimes((PFILETIME)&Idle, (PFILETIME)&Kernel, (PFILETIME)&User) == false)
	{
		return;
	}
	// Ŀ�� Ÿ�ӿ��� ���̵� Ÿ���� ���Ե�.
	ULONGLONG KernelDiff = Kernel.QuadPart - _ftProcessor_LastKernel.QuadPart;
	ULONGLONG UserDiff = User.QuadPart - _ftProcessor_LastUser.QuadPart;
	ULONGLONG IdleDiff = Idle.QuadPart - _ftProcessor_LastIdle.QuadPart;
	ULONGLONG Total = KernelDiff + UserDiff;
	ULONGLONG TimeDiff;
	if (Total == 0)
	{
		_fProcessorUser = 0.0f;
		_fProcessorKernel = 0.0f;
		_fProcessorTotal = 0.0f;
	}
	else
	{
		// Ŀ�� Ÿ�ӿ� ���̵� Ÿ���� �����Ƿ� ���� ���.
		_fProcessorTotal = (float)((double)(Total - IdleDiff) / Total * 100.0f);
		_fProcessorUser = (float)((double)UserDiff / Total * 100.0f);
		_fProcessorKernel = (float)((double)(KernelDiff - IdleDiff) / Total * 100.0f);
	}
	_ftProcessor_LastKernel = Kernel;
	_ftProcessor_LastUser = User;
	_ftProcessor_LastIdle = Idle;
	//---------------------------------------------------------
	// ������ ���μ��� ������ �����Ѵ�.
	//---------------------------------------------------------
	ULARGE_INTEGER None;
	ULARGE_INTEGER NowTime;
	//---------------------------------------------------------
	// ������ 100 ���뼼���� ���� �ð��� ���Ѵ�. UTC �ð�.
	//
	// ���μ��� ���� �Ǵ��� ����
	//
	// a = ���ð����� �ý��� �ð��� ����. (�׳� ������ ������ �ð�)
	// b = ���μ����� CPU ��� �ð��� ����.
	//
	// a : 100 = b : ���� �������� ������ ����.
	//---------------------------------------------------------
	//---------------------------------------------------------
	// ���� �ð��� �������� 100 ���뼼���� �ð��� ����,
	//---------------------------------------------------------
	GetSystemTimeAsFileTime((LPFILETIME)&NowTime);
	//---------------------------------------------------------
	// �ش� ���μ����� ����� �ð��� ����.
	//
	// �ι�°, ����°�� ����,���� �ð����� �̻��.
	//---------------------------------------------------------
	GetProcessTimes(_hProcess, (LPFILETIME)&None, (LPFILETIME)&None, (LPFILETIME)&Kernel, (LPFILETIME)&User);
	//---------------------------------------------------------
	// ������ ����� ���μ��� �ð����� ���� ���ؼ� ������ ���� �ð��� �������� Ȯ��.
	//
	// �׸��� ���� ������ �ð����� ������ ������ ����.
	//---------------------------------------------------------
	TimeDiff = NowTime.QuadPart - _ftProcess_LastTime.QuadPart;
	UserDiff = User.QuadPart - _ftProcess_LastUser.QuadPart;
	KernelDiff = Kernel.QuadPart - _ftProcess_LastKernel.QuadPart;
	Total = KernelDiff + UserDiff;
	_fProcessTotal = (float)(Total / (double)_iNumberOfProcessors / (double)TimeDiff * 100.0f);
	_fProcessKernel = (float)(KernelDiff / (double)_iNumberOfProcessors / (double)TimeDiff * 100.0f);
	_fProcessUser = (float)(UserDiff / (double)_iNumberOfProcessors / (double)TimeDiff * 100.0f);
	_ftProcess_LastTime = NowTime;
	_ftProcess_LastKernel = Kernel;
	_ftProcess_LastUser = User;
}