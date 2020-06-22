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
	errno_t err;
	err =_wfopen_s(&file, filename, L"r,ccs=UTF-16LE");

	if (err != 0)
	{
		return false;
	}
	
	while (!feof(file))
	{
		fgetws(line, 256, file);

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

	//file.open(filename);
	//
	//if (!file.is_open())
	//{
	//	return false;
	//}
	//
	//while (!file.eof())
	//{
	//	file.getline(line, 256);
	//
	//	for (WCHAR *c = line; *c; c++)
	//	{
	//		if (*c == CONFIG)
	//		{
	//			blockRead(c + 1);
	//		}
	//		else if (*c == '\t' || *c == ' ')
	//		{
	//			continue;
	//		}
	//		else if (*c == '\\'&&*(c + 1) == '\\')
	//		{
	//			break;
	//		}
	//		else
	//			break;
	//	}
	//}

	fclose(file);
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
		if (*c == ' ' || *c == '\t'||*c=='\n')
		{
			*c = '\0';
			break;
		}
	}
	block->blockName += blockName;

	fgetws(line, 256, file);
	//file.getline(line, 256);
	c = line;

	if (*c != BLOCK_START)
	{
		return;
	}

	while (1)
	{
		fgetws(line, 256, file);
		//file.getline(line, 256);
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
			else if (*c == '/')
			{
				if (*(c + 1) == '/')
				{
					value = NULL;
					break;
				}
			}
		}

		if (value != NULL)
		{
			for (; *value; value++)
			{
				if (*value == ' ' || *value == '\t'||*value=='/')
				{
					*value = '\0';
					continue;
				}
				break;
			}

			c = value;

			for (; *c; c++)
			{
				if (*c == ' ' || *c == '\t' ||*c=='\n')
				{
					*c = '\0';
					break;
				}
				else if (*c == '/')
				{
					if (*(c + 1) == '/')
					{
						*c = '\0';
						break;
					}
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