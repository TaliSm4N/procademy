#pragma once

#define CONFIG ':'
#define BLOCK_START '{'
#define BLOCK_END '}'

class TextParser
{
private:
	struct Block
	{
		std::string blockName;
		std::map<std::string, std::string> valueMap;
	};
public:
	TextParser();
	bool init(const WCHAR *filename);
	void blockRead(char *blockName);
private:
	std::ifstream file;
	std::map<std::string, Block *> blockMap;
};