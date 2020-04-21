#pragma once

#define CONFIG L':'
#define BLOCK_START L'{'
#define BLOCK_END L'}'

class TextParser
{
private:
	struct Block
	{
		std::wstring blockName;
		std::map<std::wstring, std::wstring> valueMap;
	};
public:
	TextParser();
	bool init(const WCHAR *filename);
	void blockRead(WCHAR *blockName);
	bool SetCurBlock(const WCHAR *blockName);
	bool findItem(const WCHAR *itemName, std::wstring &value);
private:
	std::wifstream file;
	std::map<std::wstring, Block *> blockMap;
	Block *curBlock;
};