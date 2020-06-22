#include <iostream>
#include <process.h>
#include <Windows.h>
#include <time.h>
#include "message.h"
#pragma comment(lib,"mysqlclient.lib")
#include "include/mysql.h"
#include "include/errmsg.h"
#include "DBConnect.h"

#include "MemoryPool.h"
#include "MemoryPoolTLS.h"
#include "LockFreeQueue.h"

unsigned __stdcall	UpdateThread(void *pParam);
unsigned __stdcall	DBThread(void *pParam);

struct DBMsg
{
	st_DBQUERY_HEADER header;
	char item[40];
};

LockFreeQueue<DBMsg *> *DBQueue;
MemoryPoolTLS<DBMsg> MsgPool;

long accountID = 1;
long playerID = 1;

long DBTPS=0;

DBConnect db;

int main()
{
	HANDLE updateThread[2];
	HANDLE dbThread;

	int waitUpdate;
	int waitDB;
	bool update = true;

	DBQueue = new LockFreeQueue<DBMsg *>(5000);

	for (int i = 0; i < 2; i++)
	{
		updateThread[i] = (HANDLE)_beginthreadex(NULL, 0, UpdateThread, NULL, 0, NULL);
	}
	dbThread = (HANDLE)_beginthreadex(NULL, 0, DBThread, NULL, 0, NULL);
	
	while (1) 
	{
		long tps = DBTPS;
		InterlockedAdd(&DBTPS, -tps);
		printf("DB TPS     : %8d\n",tps);
		printf("Queue Size : %8d\n",DBQueue->GetUseCount());
		

		if (update)
		{
			waitUpdate = WaitForMultipleObjects(2, updateThread, TRUE, 1000);
			if (waitUpdate == 0)
			{
				update = false;

			}
		}
		else
		{
			Sleep(1000);
			if (DBQueue->GetUseCount() == 0)
			{
				//test용이니깐 thread 종료같은거 신경쓰지 말자
				break;
			}
		}
		
		
	}

	

	return 0;
}

unsigned __stdcall	UpdateThread(void *pParam)
{
	DBMsg *msg;
	srand(time(NULL));
	for(int i=0;i<100000;i++)
	{
		if (DBQueue->GetFreeCount() > 2)
		{
			msg = MsgPool.Alloc();

			msg->header.Type = rand() % 3;

			switch (msg->header.Type)
			{
			case df_DBQUERY_MSG_NEW_ACCOUNT:
			{
				st_DBQUERY_MSG_NEW_ACCOUNT *account = (st_DBQUERY_MSG_NEW_ACCOUNT *)&msg->item;
				int id = InterlockedIncrement(&accountID);

				sprintf_s(account->szID, "test%d", id);
				sprintf_s(account->szPassword, "password%d", id);
				break;
			}
			case df_DBQUERY_MSG_STAGE_CLEAR:
			{
				st_DBQUERY_MSG_STAGE_CLEAR *stage = (st_DBQUERY_MSG_STAGE_CLEAR *)&msg->item;

				stage->iAccountNo = rand() % accountID + 1;
				stage->iStageID = rand() % 14 + 1;
				break;
			}
			case df_DBQUERY_MSG_PLAYER:
			{
				st_DBQUERY_MSG_PLAYER *player = (st_DBQUERY_MSG_PLAYER *)&msg->item;
				player->iAccountNo = InterlockedIncrement(&playerID);
				player->iLevel = rand() % 10;
				player->iExp = player->iLevel * 10;
				break;
			}
			default:
				i /= 0;
			}

			DBQueue->Enqueue(msg);
		}

		Sleep(3);
	}
	printf("update die\n");
	return 0;
}

void msgNewAccount(st_DBQUERY_MSG_NEW_ACCOUNT *msg,char *query)
{
	static int no = 1;
	sprintf_s(query, 1000, "INSERT INTO `account` (`accountno`, `id`, `password`) VALUES (%d,\"%s\", \"%s\")", no, msg->szID, msg->szPassword);

	no++;
}

void msgClearStage(st_DBQUERY_MSG_STAGE_CLEAR *msg, char *query)
{
	sprintf_s(query, 1000, "INSERT INTO `clearstage` (`accountno`, `stageid`) VALUES (%d,%d)", msg->iAccountNo, msg->iStageID);
}

void msgNewPlayer(st_DBQUERY_MSG_PLAYER *msg, char *query)
{
	static int no = 1;
	sprintf_s(query, 1000, "INSERT INTO `player` (`accountno`, `level`,`exp`) VALUES (%d,%d,%d)", msg->iAccountNo, msg->iLevel,msg->iExp);
	no++;
}

unsigned __stdcall	DBThread(void *pParam)
{
	db.connect("127.0.0.1", "root", "1234", "unity");
	//DB_CONNECT("127.0.0.1","root", "1234","unity");
	char test[20] = "account";
	db.Query("TRUNCATE `%s`", "account");
	db.Query("TRUNCATE `%s`", "clearstage");
	db.Query("TRUNCATE `%s`", "player");
	//DB_QUERY("TRUNCATE `account`");
	//DB_QUERY("TRUNCATE `clearstage`");
	//DB_QUERY("TRUNCATE `player`");

	DBMsg *msg;
	char query[1000];
	int i = 1;
	long dbAccountNo = 1;
	int result;

	while (1)
	{
		if (DBQueue->GetUseCount() <= 0)
			continue;

		DBQueue->Dequeue(&msg);

		switch (msg->header.Type)
		{
		case df_DBQUERY_MSG_NEW_ACCOUNT:
			msgNewAccount((st_DBQUERY_MSG_NEW_ACCOUNT *)&msg->item, query);
			break;
		case df_DBQUERY_MSG_STAGE_CLEAR:
			msgClearStage((st_DBQUERY_MSG_STAGE_CLEAR *)&msg->item, query);
			break;
		case df_DBQUERY_MSG_PLAYER:
			msgNewPlayer((st_DBQUERY_MSG_PLAYER *)&msg->item, query);
			break;
		default:
			i /= 0;
		}
		do
		{
			db.Query("BEGIN");
			result = db.Query(query);

			if (!result)
			{
				unsigned int err = db.GetLastErrNo();
				unsigned int test = 0;

				if (err == CR_SOCKET_CREATE_ERROR ||
					err == CR_CONNECTION_ERROR ||
					err == CR_CONN_HOST_ERROR ||
					err == CR_SERVER_GONE_ERROR ||
					err == CR_SERVER_HANDSHAKE_ERR ||
					err == CR_SERVER_LOST ||
					err == CR_INVALID_CONN_HANDLE)
				{
					printf("reconnect--------\n");
					db.connect();
				}
				else
				{
					result /= 0;
				}



				
			}
		} while (!result);
		db.Query("COMMIT");
		//DB_QUERY("COMMIT");
		MsgPool.Free(msg);
		InterlockedIncrement(&DBTPS);
	}
	db.Close();
	//DB_CLOSE();
}