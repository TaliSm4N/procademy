#pragma comment(lib,"mysqlclient.lib")
#include "include/mysql.h"
#include "include/errmsg.h"

#include "DBConnect.h"

DBConnect *connecter=nullptr;
DBConnect *DBConnect::_db = nullptr;


DBConnect::DBConnect()
	:connection(NULL),result(NULL)
{
	mysql_init(&conn);
}

DBConnect *DBConnect::GetInstance()
{
	if (_db == nullptr)
	{
		_db = new DBConnect();
	}

	return _db;
}

bool DBConnect::connect(const char *host, const char *db)
{
	connection = mysql_real_connect(&conn, "127.0.0.1", host, "1234", db, 3306, (char *)NULL, 0);
	if (connection == NULL)
		return false;

	mysql_set_character_set(connection, "utf8");

	return true;
}

const char *DBConnect::error()
{
	return mysql_error(connection);
}

unsigned int DBConnect::errorno()
{
	return mysql_errno(connection);
}

int DBConnect::Query(const char *query)
{
	return mysql_query(connection, query);
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