#include <stdio.h>
#include <Pdh.h>
#include <PdhMsg.h>
#include "CPUUsage.h"
#pragma comment(lib,"Pdh.lib")

int main(void)
{
	//HWND hwnd;
	//DWORD pid;
	//hwnd = FindWindow(0, L"echo_library");
	//if (hwnd == 0)
	//{
	//	system("pause");
	//	return 0;
	//}
	//GetWindowThreadProcessId(hwnd,&pid);
	//HANDLE hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, pid);
	CCpuUsage cpuMonitor; 

	while (true)
	{
		Sleep(1000);
		

		cpuMonitor.Update();
		
		printf("processor Total  %f%%\n", cpuMonitor.ProcessorTotal());
		printf("processor User   %f%%\n", cpuMonitor.ProcessorUser());
		printf("processor Kernel %f%%\n", cpuMonitor.ProcessKernel());
		printf("process   Total  %f%%\n", cpuMonitor.ProcessTotal());
		printf("process   User   %f%%\n", cpuMonitor.ProcessUser());
		printf("process   Kernel %f%%\n", cpuMonitor.ProcessKernel());
		printf("process Total private Bytes %llu\n", cpuMonitor.PrivateByteTotal());
		printf("process       private Bytes %llu\n", cpuMonitor.PrivateByte());
		printf("process Total nonPaged Bytes %llu\n", cpuMonitor.ProcessNonPagedByteTotal());
		printf("process       nonPaged Bytes %llu\n", cpuMonitor.ProcessNonPagedByte());
		printf("memory available Mbytes %llu\n", cpuMonitor.AvailableMBytes());
		printf("memory nonpaged   bytes %llu\n", cpuMonitor.PoolNonpagedBytes());
	}

	return 0;
}