#pragma once

#define MAX_COL 100

class CSVConverter
{
public:
	CSVConverter();
	bool SetPath(const char *path);
	bool SetPath(const WCHAR *path);
	bool SetDest(const char *dest);
	bool SetDest(const WCHAR *dest);
	bool SetTable(const char *table);
	bool SetTable(const WCHAR *table);
	bool SetSchema(const char *schema);
	bool SetSchema(const WCHAR *schema);
	
	bool Convert();
private:
	void ReadLine(std::vector<std::string> *strVec);
	void MakeQuery(std::vector<std::string> *strVec, std::vector<std::string> *query);
	void CutComma(std::string *comStr, std::vector<std::string> *res);
	void MakeQueryFile(std::vector<std::string> *query);
private:
	char _path[MAX_PATH];
	char _dest[MAX_PATH];
	char _table[MAX_PATH];
	char _schema[MAX_PATH];
};