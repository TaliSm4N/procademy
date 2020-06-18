#include <Windows.h>
#include <cstring>
#include <fstream>
#include <map>
#include "TextParser.h"

TextParser::TextParser()
	:curBlock(NULL)
{
}

bool TextParser::init(const WCHAR *filename)
{
	WCHAR line[256];
	file.open(filename);

	if (!file.is_open())
	{
		return false;
	}

	while (!file.eof())
	{
		file.getline(line, 256);

		for (WCHAR *c = line; *c; c++)
		{
			if (*c == CONFIG)
			{
				blockRead(c + 1);
			}
			else if (*c == '\t' || *c == ' ')
			{
				continue;
			}
			else if (*c == '\\'&&*(c + 1) == '\\')
			{
				break;
			}
			else
				break;
		}
	}


	return true;
}

void TextParser::blockRead(WCHAR *blockName)
{
	Block *block = new Block();
	//int len = strlen(blockName);
	WCHAR line[256];
	WCHAR *c;
	WCHAR *key=NULL;
	WCHAR *value=NULL;
	std::wstring str;
	
	

	c = blockName;
	for (; *c; c++)
	{
		if (*c == ' ' || *c == '\t')
		{
			*c = '\0';
			break;
		}
	}
	block->blockName += blockName;

	file.getline(line, 256);
	c = line;

	if (*c != BLOCK_START)
	{
		return;
	}

	while (1)
	{
		file.getline(line, 256);
		c = line;

		if (*c == BLOCK_END)
			break;

		for (; *c; c++)
		{
			if (*c == ' ' || *c == '\t')
				continue;
			break;
		}
		
		key = c;
		value = NULL;

		for (; *c; c++)
		{
			if (*c == '=')
			{
				*c = '\0';
				value = c + 1;
				break;
			}
			else if (*c == ' ' || *c == '\t')
			{
				*c = '\0';
			}
		}

		if (value != NULL)
		{
			for (; *value; value++)
			{
				if (*value == ' ' || *value == '\t'||*value=='\"')
				{
					*value = '\0';
					continue;
				}
				break;
			}

			c = value;

			for (; *c; c++)
			{
				if (*c == ' ' || *c == '\t' || *c == '\"')
				{
					*c = '\0';
					break;
				}
			}

			block->valueMap.insert(std::make_pair(key, value));
		}

	}

	blockMap.insert(std::make_pair(block->blockName, block));
}

bool TextParser::SetCurBlock(const WCHAR *blockName)
{

	auto iter =blockMap.find(blockName);
	
	
	if (iter == blockMap.end())
	{
		return false;
	}

	curBlock = iter->second;
}

bool TextParser::findItem(const WCHAR *itemName, std::wstring &value)
{
	if (curBlock == NULL)
		return false;

	auto iter =curBlock->valueMap.find(itemName);

	if (iter == curBlock->valueMap.end())
		return false;

	value += iter->second;

	return true;
}