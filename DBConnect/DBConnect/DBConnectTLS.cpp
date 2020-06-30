#pragma comment(lib,"winmm.lib")
#include <strsafe.h>
#pragma comment(lib,"mysqlclient.lib")
#include <list>
#include "include/mysql.h"
#include "include/errmsg.h"

#include "DBConnect.h"
#include "DBConnectTLS.h"


DBConnectTLS::DBConnectTLS()
{
	_tlsIndex = TlsAlloc();
	InitializeSRWLock(&listLock);
}

void DBConnectTLS::init(const char *ip, const char *user, const char *password, const char *db, int port)
{
	strcpy_s(_szDBIP, ip);
	strcpy_s(_szDBUser, user);
	strcpy_s(_szDBPassword, password);
	strcpy_s(_szDBName, db);
	_iDBPort = port;
}

bool DBConnectTLS::IsConnect()
{
	DBConnect *Mydb = GetDB();

	return Mydb->IsConnect();
}

DBConnect *DBConnectTLS::GetDB()
{
	DBConnect *db = (DBConnect *)TlsGetValue(_tlsIndex);

	if (db == NULL)
	{
		db = new DBConnect();
		TlsSetValue(_tlsIndex, db);
		AcquireSRWLockExclusive(&listLock);
		_dbList.push_back(db);
		ReleaseSRWLockExclusive(&listLock);
	}
	
	return db;
}

DBConnectTLS::~DBConnectTLS()
{
	AcquireSRWLockExclusive(&listLock);
	for (auto iter = _dbList.begin(); iter != _dbList.end();)
	{
		(*iter)->Close();
		delete (*iter);
		iter = _dbList.erase(iter);
	}
	ReleaseSRWLockExclusive(&listLock);
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

bool DBConnectTLS::connect(const char *ip, const char *user, const char *password, const char *db, int port)
{
	
	DBConnect *Mydb = GetDB();

	return Mydb->connect(ip,user, password, db, port);
}

bool DBConnectTLS::connect()
{
	DBConnect *Mydb = GetDB();

	return Mydb->connect(_szDBIP, _szDBUser, _szDBPassword, _szDBName, _iDBPort);
}

const char *DBConnectTLS::GetLastError()
{
	DBConnect *Mydb = GetDB();

	return Mydb->GetLastError();
}

unsigned int DBConnectTLS::GetLastErrNo()
{
	DBConnect *Mydb = GetDB();

	return Mydb->GetLastErrNo();
}

bool DBConnectTLS::Query(const char *query, ...)
{
	DBConnect *Mydb = GetDB();

	char myQuery[eQUERY_MAX_LEN];
	va_list va;

	va_start(va, query);

	StringCchVPrintfA(myQuery, eQUERY_MAX_LEN, query, va);

	va_end(va);


	return Mydb->Query(myQuery);
}

MYSQL_RES *DBConnectTLS::GetResults()
{
	DBConnect *Mydb = GetDB();

	return Mydb->GetResults();
}
MYSQL_ROW DBConnectTLS::FetchRow(MYSQL_RES *res)
{
	DBConnect *Mydb = GetDB();

	return Mydb->FetchRow(res);
}
MYSQL_ROW DBConnectTLS::GetRow()
{
	DBConnect *Mydb = GetDB();

	return Mydb->GetRow();
}

void DBConnectTLS::FreeResult()
{
	DBConnect *Mydb = GetDB();

	Mydb->FreeResult();
}
void DBConnectTLS::FreeResult(MYSQL_RES *res)
{
	DBConnect *Mydb = GetDB();

	Mydb->FreeResult(res);
}

void DBConnectTLS::Close()
{
	DBConnect *Mydb = GetDB();

	Mydb->Close();
}

int DBConnectTLS::GetInsertID()
{
	DBConnect *Mydb = GetDB();

	return Mydb->GetInsertID();
}

DWORD DBConnectTLS::GetLastQueryTime()
{
	DBConnect *Mydb = GetDB();

	return Mydb->GetLastQueryTime();
}