#pragma comment(lib,"winmm.lib")
#include <strsafe.h>
#pragma comment(lib,"mysqlclient.lib")
#include "include/mysql.h"
#include "include/errmsg.h"

#include "DBConnect.h"

//DBConnect *connecter=nullptr;
//DBConnect *DBConnect::_db = nullptr;

void WCharToChar(const WCHAR *wChr, char *chr)
{
	int len = wcslen(wChr) + 1;

	wcstombs_s(NULL, chr, len, wChr, len);
}



DBConnect::DBConnect()
	:connection(NULL),result(NULL)
{
	mysql_init(&conn);
}

//DBConnect *DBConnect::GetInstance()
//{
//	if (_db == nullptr)
//	{
//		_db = new DBConnect();
//	}
//
//	return _db;
//}

bool DBConnect::connect(const char *ip,const char *user,const char *password, const char *db,int port)
{
	strcpy_s(_szDBIP, ip);
	strcpy_s(_szDBUser, user);
	strcpy_s(_szDBPassword, password);
	strcpy_s(_szDBName, db);
	_iDBPort = port;
	connection = mysql_real_connect(&conn, _szDBIP, _szDBUser, _szDBPassword, _szDBName, _iDBPort, (char *)NULL, 0);
	if (connection == NULL)
		return false;

	mysql_set_character_set(connection, "utf8");

	return true;
}

bool DBConnect::connect()
{
	connection = mysql_real_connect(&conn, _szDBIP, _szDBUser, _szDBPassword, _szDBName, _iDBPort, (char *)NULL, 0);
	if (connection == NULL)
	{
		strcpy_s(_LastErrorMsg, mysql_error(connection));
		_iLastError = mysql_errno(connection);
		
		return false;
	}

	mysql_set_character_set(connection, "utf8");

	return true;
}

const char *DBConnect::GetLastError()
{
	return _LastErrorMsg;
}

unsigned int DBConnect::GetLastErrNo()
{
	return _iLastError;
}

bool DBConnect::Query(const char *query, ...)
{
	va_list va;

	va_start(va, query);

	StringCchVPrintfA(_szQueryUTF8, eQUERY_MAX_LEN, query,va);

	char *t =va_arg(va, char *);

	va_end(va);

	DWORD startTime = timeGetTime();
	int result = mysql_query(connection, _szQueryUTF8);
	DWORD endTime = timeGetTime();
	if (result != 0)
	{
		strcpy_s(_LastErrorMsg, mysql_error(connection));
		_iLastError = mysql_errno(connection);

		return false;
	}

	_queryTime = endTime - startTime;

	return true;
}

MYSQL_RES *DBConnect::GetResults()
{
	return mysql_store_result(connection);
}
MYSQL_ROW DBConnect::FetchRow(MYSQL_RES *res)
{
	MYSQL_ROW ret = mysql_fetch_row(res);
	if (ret == NULL)
	{
		mysql_free_result(res);
	}

	return ret;
}
MYSQL_ROW DBConnect::GetRow()
{
	MYSQL_ROW ret;
	if (result == NULL)
	{
		result=mysql_store_result(connection);
	}
	ret = mysql_fetch_row(result);

	if (ret == NULL)
	{
		mysql_free_result(result);
		result = NULL;
	}

	return ret;
}

void DBConnect::FreeResult()
{
	mysql_free_result(result);
	result = NULL;
}
void DBConnect::FreeResult(MYSQL_RES *res)
{
	mysql_free_result(res);
}

void DBConnect::Close()
{
	mysql_close(connection);
	connection = NULL;
}

int DBConnect::GetInsertID()
{
	return mysql_insert_id(connection);
}