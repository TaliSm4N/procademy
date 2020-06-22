#include <Windows.h>
#include <cstring>
#include <fstream>
#include <map>
#include "TextParser.h"

int main()
{
	TextParser parser;

	parser.init(L"test.txt");

	parser.SetCurBlock(L"SERVER");

	std::wstring str;


	system("pause");
}