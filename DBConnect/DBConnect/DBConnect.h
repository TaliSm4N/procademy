#pragma once

//#define DB_CONNECT_PORT(IP,HOST,PASSWORD,DB,PORT) (connecter->GetInstance()->connect(IP,HOST,PASSWORD,DB,PORT))
//#define DB_CONNECT(IP,HOST,PASSWORD,DB) (connecter->GetInstance()->connect(IP,HOST,PASSWORD,DB))
//#define DB_QUERY(QUERY,...) (connecter->GetInstance()->Query(QUERY,##__VA_ARGS__))
//#define DB_ERROR() (connecter->GetInstance()->error())
//#define DB_ERRORNO() (connecter->GetInstance()->errorno())
//#define DB_GET_RESULTS() (connecter->GetInstance()->GetResults())
//#define DB_FETCH_ROW(RES) (connecter->GetInstance()->FetchRow(RES))
//#define DB_ROW() (connecter->GetInstance()->GetRow())
////#define DB_FREE_RESULT() (connecter->GetInstance()->FreeResult())
//#define DB_FREE_RESULT(RES) (connecter->GetInstance()->FreeResult(RES))
//#define DB_CLOSE() (connecter->Close())
//#define DB_INSERT_ID()

class DBConnect
{
public:

	enum en_DB_CONNECTOR
	{
		eQUERY_MAX_LEN = 2048

	};

	DBConnect();
	//static DBConnect *GetInstance();
	bool connect(const char *ip, const char *user, const char *password, const char *db,int port =3306);
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
private:
private:
	MYSQL conn;
	MYSQL *connection;

	MYSQL_RES *result;

	char		_szDBIP[16];
	char		_szDBUser[64];
	char		_szDBPassword[64];
	char		_szDBName[64];
	int			_iDBPort;

	//WCHAR		_szQuery[eQUERY_MAX_LEN];
	char		_szQueryUTF8[eQUERY_MAX_LEN];

	int			_iLastError;
	char		_LastErrorMsg[128];
};

//extern DBConnect *connecter;