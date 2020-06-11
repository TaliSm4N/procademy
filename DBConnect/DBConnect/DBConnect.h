#pragma once

#define DB_CONNECT(HOST,DB) (connecter->GetInstance()->connect(HOST,DB))
#define DB_QUERY(QUERY) (connecter->GetInstance()->Query(QUERY))
#define DB_ERROR() (connecter->GetInstance()->error())
#define DB_ERRORNO() (connecter->GetInstance()->errorno())
#define DB_GET_RESULTS() (connecter->GetInstance()->GetResults())
#define DB_FETCH_ROW(RES) (connecter->GetInstance()->FetchRow(RES))
#define DB_ROW() (connecter->GetInstance()->GetRow())
//#define DB_FREE_RESULT() (connecter->GetInstance()->FreeResult())
#define DB_FREE_RESULT(RES) (connecter->GetInstance()->FreeResult(RES))
#define DB_CLOSE() (connecter->GetInstance()->Close())
#define DB_INSERT_ID()

class DBConnect
{
public:
	static DBConnect *GetInstance();
	bool connect(const char *host,const char *db);
	const char *error();
	unsigned int errorno();
	int Query(const char *query);
	MYSQL_RES *GetResults();
	MYSQL_ROW FetchRow(MYSQL_RES *res);
	MYSQL_ROW GetRow();
	void FreeResult();
	void FreeResult(MYSQL_RES *);
	void Close();
	int GetInsertID();
private:
	DBConnect();
private:
	static DBConnect *_db;
	MYSQL conn;
	MYSQL *connection;

	MYSQL_RES *result;
};

extern DBConnect *connecter;