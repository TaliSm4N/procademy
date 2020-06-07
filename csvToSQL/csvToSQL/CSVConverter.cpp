#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>
#include "CSVConverter.h"

void WCharToChar(const WCHAR *wChr, char *chr)
{
	int len = wcslen(wChr) + 1;

	wcstombs_s(NULL, chr, len, wChr, len);
}

CSVConverter::CSVConverter()
{}

bool CSVConverter::SetPath(const char *path)
{
	if (strlen(path) > MAX_PATH)
		return false;

	strcpy_s(_path, path);

	return true;
}

bool CSVConverter::SetPath(const WCHAR *path)
{
	if (wcslen(path) > MAX_PATH)
		return false;

	WCharToChar(path, _path);

	return true;
}

bool CSVConverter::SetDest(const char *dest)
{
	if (strlen(dest) > MAX_PATH)
		return false;

	strcpy_s(_dest, dest);

	return true;
}

bool CSVConverter::SetDest(const WCHAR *dest)
{
	if (wcslen(dest) > MAX_PATH)
		return false;

	WCharToChar(dest, _dest);

	return true;
}

bool CSVConverter::SetTable(const char *table)
{
	if (strlen(table) > MAX_PATH)
		return false;

	strcpy_s(_table, table);

	return true;
}
bool CSVConverter::SetTable(const WCHAR *table)
{
	if (wcslen(table) > MAX_PATH)
		return false;

	WCharToChar(table, _table);

	return true;
}
bool CSVConverter::SetSchema(const char *schema)
{
	if (strlen(schema) > MAX_PATH)
		return false;

	strcpy_s(_schema, schema);

	return true;
}
bool CSVConverter::SetSchema(const WCHAR *schema)
{
	if (wcslen(schema) > MAX_PATH)
		return false;

	WCharToChar(schema, _schema);

	return true;
}

bool CSVConverter::Convert()
{
	std::vector<std::string> strVec;
	std::vector<std::string> query;

	ReadLine(&strVec);
	MakeQuery(&strVec, &query);
	MakeQueryFile(&query);

	return true;
}

void CSVConverter::ReadLine(std::vector<std::string> *strVec)
{
	std::string str="";
	char buf[512];
	char *cur;
	FILE *f;
	fopen_s(&f, _path, "r");

	if (f == NULL)
		return;

	while (fgets(buf, 512, f) != NULL)
	{
		cur = strchr(buf, '\n');
		if (cur != NULL)
			*cur = '\0';
		str += buf;
		if(cur!=NULL)
		{
			strVec->push_back(str);
			str = "";
		}
	}

	fclose(f);

}

void CSVConverter::MakeQuery(std::vector<std::string> *strVec, std::vector<std::string> *query)
{
	std::string *str = &(*strVec)[0];
	std::vector<std::string> attr;
	std::vector<std::string> line;
	std::string q;
	char buf[512];
	
	
	CutComma(str,&attr);

	//printf("BEGIN;\n");

	query->push_back(std::string("BEGIN;"));


	for (int i = 1; i < strVec->size(); i++)
	{
		sprintf_s(buf,"INSERT INTO `%s`.`%s` (",_schema,_table);
		q += buf;
		for (int j = 0; j < attr.size()-1; j++)
		{
			sprintf_s(buf,"`%s`,",attr[j].c_str());
			q += buf;
		}
		sprintf_s(buf,"`%s`) VALUES (", attr[attr.size() - 1]);
		q += buf;

		str = &(*strVec)[i];
		CutComma(str, &line);

		for (int j = 0; j < line.size() - 1; j++)
		{
			sprintf_s(buf,"\'%s\',", line[j].c_str());
			q += buf;
		}
		sprintf_s(buf,"\'%s\');", line[line.size() - 1]);
		q += buf;

		line.clear();
		query->push_back(q);
		q.clear();
	}


	query->push_back(std::string("COMMIT;"));

	//printf("COMMIT;\n");
}

void CSVConverter::CutComma(std::string *comStr,std::vector<std::string> *res)
{
	std::string substring;
	int current;
	int prev=0;

	current = comStr->find(',');

	while (current != std::string::npos)
	{
		substring = comStr->substr(prev, current - prev);
		//printf("%s - ",substring.c_str());
		res->push_back(substring);
		prev = current + 1;
		current = comStr->find(',', prev);
	}

	substring = comStr->substr(prev, current - prev);
	res->push_back(substring);

	//printf("%s\n", substring.c_str());
}

void CSVConverter::MakeQueryFile(std::vector<std::string> *query)
{
	std::string *str;
	char buf[512]="test";
	char *cur;
	FILE *f;
	fopen_s(&f, _dest, "w");
	rewind(f);

	//fwrite(buf, 5, 1, f);
	for (int i = 0; i < query->size(); i++)
	{
		str = &(*query)[i];
		
		fprintf(f,"%s\n", str->c_str());
		//fwrite(str->c_str(), str->size() + 1, 1, f);
		
	}

	fclose(f);
}