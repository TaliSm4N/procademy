#include <iostream>
#include <Windows.h>
#include <vector>
#include "CSVConverter.h"

int main()
{
	printf("hello\n");
	CSVConverter converter;

	converter.SetPath("data.csv");
	converter.SetDest("data.sql");
	converter.SetSchema("csv_sql");
	converter.SetTable("data1");
	converter.Convert();

	system("pause");
}