#include <iostream>
#include <Windows.h>
#include "Profiler.h"

void test1();
void test2();
void test3();
void test4();
void test5();

int main()
{
	for (int i = 0; i < 10; i++)
	{
		test1();
		test2();
		test3();
		test4();
		test5();
	}

	for (int i = 0; i < 5; i++)
	{
		test1();
		
		test3();
		
		test5();
	}

	for (int i = 0; i < 1; i++)
	{
		test3();
	}

	ProfileDataOutText(L"tt");
	system("pause");
}

void test1()
{
	PRO_BEGIN(L"TEST1");
	Sleep(55);
	PRO_END(L"TEST1");
}
void test2()
{
	PRO_BEGIN(L"TEST2");
	Sleep(0);
	PRO_END(L"TEST2");
}
void test3()
{
	PRO_BEGIN(L"TEST3");
	Sleep(22);
	PRO_END(L"TEST3");
}
void test4()
{
	PRO_BEGIN(L"TEST4");
	Sleep(100);
	PRO_END(L"TEST4");
}
void test5()
{
	PRO_BEGIN(L"TEST5");
	Sleep(300);
	PRO_END(L"TEST5");
}