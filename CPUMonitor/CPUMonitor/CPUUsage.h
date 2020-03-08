#ifndef __CPU_USAGE_H__
#define __CPU_USAGE_H__
/////////////////////////////////////////////////////////////////////////////
// CCpuUsage CPUTime(); // CPUTime(hProcess)
//
// while ( 1 )
// {
// CPUTIme.UpdateCpuTime();
// wprintf(L"Processor:%f / Process:%f \n", CPUTime.ProcessorTotal(), CPUTime.ProcessTotal());
// wprintf(L"ProcessorKernel:%f / ProcessKernel:%f \n", CPUTime.ProcessorKernel(), CPUTime.ProcessKernel());
// wprintf(L"ProcessorUser:%f / ProcessUser:%f \n", CPUTime.ProcessorUser(), CPUTime.ProcessUser());
// Sleep(1000);
// }
/////////////////////////////////////////////////////////////////////////////
class CCpuUsage
{
public:
	//----------------------------------------------------------------------
	// ������, Ȯ�δ�� ���μ��� �ڵ�. ���Է½� �ڱ� �ڽ�.
	//----------------------------------------------------------------------
	CCpuUsage(HANDLE hProcess = INVALID_HANDLE_VALUE);

	void Init();
	void Update();
	void UpdateCpuTime(void);
	float ProcessorTotal(void) { return _fProcessorTotal; }
	float ProcessorUser(void) { return _fProcessorUser; }
	float ProcessorKernel(void) { return _fProcessorKernel; }
	float ProcessTotal(void) { return _fProcessTotal; }
	float ProcessUser(void) { return _fProcessUser; }
	float ProcessKernel(void) { return _fProcessKernel; }

	LONGLONG PrivateByte() { return _privateBytes; }
	LONGLONG PrivateByteTotal() { return _privateBytesTotal; }

	LONGLONG ProcessNonPagedByte() { return _processNonPagedBytes; }
	LONGLONG ProcessNonPagedByteTotal() { return _processNonPagedBytesTotal; }

	LONGLONG AvailableMBytes() { return _availableMBytes; }
	LONGLONG PoolNonpagedBytes() { return _poolNonpagedBytes; }
private:
	HANDLE _hProcess;
	WCHAR _processName[MAX_PATH];
	int _iNumberOfProcessors;
	float _fProcessorTotal;
	float _fProcessorUser;
	float _fProcessorKernel;
	float _fProcessTotal;
	float _fProcessUser;
	float _fProcessKernel;
	ULARGE_INTEGER _ftProcessor_LastKernel;
	ULARGE_INTEGER _ftProcessor_LastUser;
	ULARGE_INTEGER _ftProcessor_LastIdle;
	ULARGE_INTEGER _ftProcess_LastKernel;
	ULARGE_INTEGER _ftProcess_LastUser;
	ULARGE_INTEGER _ftProcess_LastTime;

	//��������
	LONGLONG _privateBytes;
	LONGLONG _privateBytesTotal;

	LONGLONG _processNonPagedBytes;
	LONGLONG _processNonPagedBytesTotal;

	LONGLONG _availableMBytes;
	LONGLONG _poolNonpagedBytes;

	PDH_HQUERY _query;
	PDH_HCOUNTER _privateCounter;
	PDH_HCOUNTER _privateTotalCounter;

	PDH_HCOUNTER _processNonPagedCounter;
	PDH_HCOUNTER _processNonPagedTotalCounter;

	PDH_HCOUNTER _availableMByteCounter;
	PDH_HCOUNTER _poolNonpagedByteCounter;
};
#endif