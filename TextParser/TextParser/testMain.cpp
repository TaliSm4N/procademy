#include <Windows.h>
#include <cstring>
#include <fstream>
#include <map>
#include "TextParser.h"

int main()
{
	TextParser parser;

	parser.init(L"test.txt");

	system("pause");
}