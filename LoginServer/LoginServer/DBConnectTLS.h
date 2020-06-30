#pragma once

class DBConnectTLS
{
public:

	enum en_DB_CONNECTOR
	{
		eQUERY_MAX_LEN = 2048

	};

	DBConnectTLS();
	~DBConnectTLS();
	//static DBConnect *GetInstance();
	void init(const char *ip, const char *user, const char *password, const char *db, int port = 3306);
	bool connect(const char *ip, const char *user, const char *password, const char *db, int port = 3306);
	bool connect();
	const char *GetLastError();
	unsigned int GetLastErrNo();
	bool Query(const char *query, ...);
	MYSQL_RES *GetResults();
	MYSQL_ROW FetchRow(MYSQL_RES *res);
	MYSQL_ROW GetRow();
	void FreeResult();
	void FreeResult(MYSQL_RES *);
	void Close();
	int GetInsertID();
	DWORD GetLastQueryTime();
	bool IsConnect();
	DBConnect *GetDB();
private:
private:

	char		_szDBIP[16];
	char		_szDBUser[64];
	char		_szDBPassword[64];
	char		_szDBName[64];
	int			_iDBPort;

	DWORD _tlsIndex;
	std::list<DBConnect *>_dbList;
	SRWLOCK listLock;
};