#include <WinSock2.h>
#include <unordered_map>

#include <strsafe.h>
#include <list>
#pragma comment(lib,"mysqlclient.lib")
#include "include/mysql.h"
#include "include/errmsg.h"

#include "DBConnect.h"
#include "DBConnectTLS.h"
#include "CommonProtocol.h"
#include "LanServerLib.h"
#include "NetServerLib.h"
#include "ServerConnector.h"
#include "loginServer.h"

LoginServer::LoginServer()
	:loginMin(-1)
{
	_connecter.Init(this);
	InitializeSRWLock(&waiterLock);
	
	//InitializeSRWLock(&dbLock);
}

bool LoginServer::Start(int port, int workerCnt, bool nagle, int maxUser, bool monitoring)
{
	//accountDB.connect("127.0.0.1", "root", "1234", "accountdb");
	//accountDB.Query("UPDATE `status` SET `status` = 0");
	_waiterPool = new MemoryPoolTLS<Waiter>(maxUser, true);
	accountDB.init("127.0.0.1", "root", "1234", "accountdb");

	_connecter.Start(5000, 5, true, 10);

	return CNetServer::Start(port,workerCnt,nagle,maxUser,monitoring);
}

bool LoginServer::Start()
{
	//accountDB.connect("127.0.0.1", "root", "1234", "accountdb");
	//accountDB.Query("UPDATE `status` SET `status` = 0");

	_waiterPool = new MemoryPoolTLS<Waiter>(GetMaxUser(), true);
	accountDB.init("127.0.0.1", "root", "1234", "accountdb");
	_connecter.Start();



	return CNetServer::Start();
}

bool LoginServer::Config(const WCHAR *configFile)
{
	if (!CNetServer::Config(configFile, L"SERVER"))
		return false;
	if (!_connecter.Config(configFile, L"CONNECTOR"))
		return false;
	TextParser parser;
	std::wstring str;

	if (!parser.init(configFile))
		return false;

	if (!parser.SetCurBlock(L"LoginServer"))
		return false;

	if (!parser.findItem(L"CHAT_IP", str))
		return false;
	wcscpy_s(ChatIP, str.c_str());
	str.clear();

	if (!parser.findItem(L"CHAT_PORT", str))
		return false;
	ChatPort = std::stoi(str);
	str.clear();
	return true;
}

bool LoginServer::OnConnectionRequest(WCHAR *ClientIP, int Port)
{
	return true;
}
void LoginServer::OnRecv(DWORD sessionID, Packet *p)
{
	WORD type;

	*p >> type;

	switch (type)
	{
	case en_PACKET_CS_LOGIN_REQ_LOGIN:
		LoginRequest(sessionID,p);
		break;
	}
}
void LoginServer::OnSend(DWORD sessionID, int sendsize)
{}
void LoginServer::OnClientJoin(DWORD sessionID)
{

}
void LoginServer::OnClientLeave(DWORD sessionID)
{
	AcquireSRWLockExclusive(&waiterLock);
	auto iter = _WaiterMap.find(sessionID);

	if (iter == _WaiterMap.end())
	{
		ReleaseSRWLockExclusive(&waiterLock);
		return;
	}
	_WaiterMap.erase(iter);
	ReleaseSRWLockExclusive(&waiterLock);
}
void LoginServer::OnError(int errorcode, WCHAR *)
{}

bool LoginServer::LoginRequest(DWORD sessionID, Packet *p)
{
	INT64 accountNo;
	char sessionKey[64];
	int version;
	WCHAR id[20];
	WCHAR nick[20];
	int status;
	long startTime=timeGetTime();

	*p >> accountNo;
	p->GetData(sessionKey, 64);
	*p >> version;

	//AcquireSRWLockExclusive(&dbLock);

	//accountDB.connect();
	DBConnect *db = accountDB.GetDB();

	//if (!db->IsConnect())
	//{
	//	accountDB.connect();
	//	
	//}
	
	//bool re = accountDB.Query("SELECT sessionkey, userid, usernick, status FROM v_account where accountno = %ld", accountNo);
	bool re = db->Query("SELECT sessionkey, userid, usernick, status FROM v_account where accountno = %ld", accountNo);

	if (!re)
	{
		Packet *pk = MakeLoginResponse(accountNo, dfLOGIN_STATUS_FAIL, NULL, NULL);
		SendPacket(sessionID, pk);
		Packet::Free(pk);

		LOG(L"query", LOG_ERROR, L"query Fail account(%d) session(%d) : %s (%d)",accountNo, sessionID, db->GetLastError(), db->GetLastErrNo());
		//accountDB.FreeResult();

		//ReleaseSRWLockExclusive(&dbLock);
		return false;
	}


	DWORD time = db->GetLastQueryTime();
	//DWORD time = accountDB.GetLastQueryTime();

	//5초 이상걸린 쿼리 기록
	if (time > 5000)
	{
		LOG(L"query", LOG_ERROR, L"Query time over %d ms : accountNo = %ld", time, accountNo);
	}

	MYSQL_ROW row = db->GetRow();
	//MYSQL_ROW row = accountDB.GetRow();

	if (row == NULL)
	{
		//실패
		Packet *pk = MakeLoginResponse(accountNo, dfLOGIN_STATUS_FAIL, NULL, NULL);
		SendPacket(sessionID, pk);
		Packet::Free(pk);

		db->FreeResult();
		//accountDB.FreeResult();

		//ReleaseSRWLockExclusive(&dbLock);
		return false;
	}

	if (row[1] == NULL)
	{
		Packet *pk = MakeLoginResponse(accountNo, dfLOGIN_STATUS_ACCOUNT_MISS, NULL, NULL);
		SendPacket(sessionID, pk);
		Packet::Free(pk);

		db->FreeResult();
		//accountDB.FreeResult();

		//ReleaseSRWLockExclusive(&dbLock);
		return false;
	}


	//status 체크
	if (row[3] == NULL)
	{
		Packet *pk = MakeLoginResponse(accountNo, dfLOGIN_STATUS_STATUS_MISS, NULL, NULL);
		SendPacket(sessionID, pk);
		Packet::Free(pk);

		db->FreeResult();
		//accountDB.FreeResult();

		//ReleaseSRWLockExclusive(&dbLock);
		return false;
	}
	else
	{
		if (atoi(row[3]) != 0)
		{
			Packet *pk = MakeLoginResponse(accountNo, dfLOGIN_STATUS_GAME, NULL, NULL);
			
			SendPacket(sessionID, pk);
			Packet::Free(pk);

			db->FreeResult();
			//accountDB.FreeResult();

			//ReleaseSRWLockExclusive(&dbLock);
			return false;
		}
	}


	//dummy일 때 조건 생략
	if (accountNo > 100000)
	{
		if (strcmp(sessionKey, row[0]) != 0)
		{
			//실패
			Packet *pk = MakeLoginResponse(accountNo, dfLOGIN_STATUS_FAIL, NULL, NULL);

			SendPacket(sessionID, pk);
			Packet::Free(pk);

			db->FreeResult();
			//accountDB.FreeResult();

			//ReleaseSRWLockExclusive(&dbLock);
			return false;
		}
	}

	Waiter *waiter = _waiterPool->Alloc();
	waiter->SessionID = sessionID;
	waiter->AccountNo = accountNo;
	waiter->startTime = startTime;
	wsprintf(waiter->id, L"%s", row[1]);
	wsprintf(waiter->nick, L"%s", row[2]);
	
	AcquireSRWLockExclusive(&waiterLock);
	_WaiterMap.insert(std::make_pair(sessionID, waiter));
	ReleaseSRWLockExclusive(&waiterLock);




	//Packet *pk = MakeLoginResponse(accountNo, dfLOGIN_STATUS_OK, waiter->id, waiter->nick);
	//SendPacket(sessionID, pk);
	//Packet::Free(pk);
	//
	//accountDB.FreeResult();
	//
	//return true;


	//-------------------------------------------------
	//status 체크 및 변경과 같은 것 추가해야할 부분
	//-------------------------------------------------

	if (!_connecter.ClientLoginReq_ss(accountNo, sessionKey, sessionID, dfSERVER_TYPE_CHAT))
	{
		//실패
		Packet *pk = MakeLoginResponse(accountNo, dfLOGIN_STATUS_NOSERVER, NULL, NULL);
		SendPacket(sessionID, pk);
		Packet::Free(pk);

		db->FreeResult();
		//accountDB.FreeResult();

		//ReleaseSRWLockExclusive(&dbLock);
		return false;
	}

	db->FreeResult();
	//accountDB.FreeResult();

	//ReleaseSRWLockExclusive(&dbLock);

	return true;
	//printf("%s %s\n", row[0], row[1]);
}

Packet *LoginServer::MakeLoginResponse(INT64 accountNo, BYTE status, WCHAR *id, WCHAR *nick)
{
	Packet *p = Packet::Alloc();

	*p << (WORD)en_PACKET_CS_LOGIN_RES_LOGIN << accountNo << status;

	if (id == NULL)
	{
		p->MoveWritePos(20 * sizeof(WCHAR));
	}
	else
	{
		p->PutData((char *)id, 20 * sizeof(WCHAR));
	}
	
	if (nick == NULL)
	{
		p->MoveWritePos(20 * sizeof(WCHAR));
	}
	else
	{
		p->PutData((char *)nick, 20 * sizeof(WCHAR));
	}

	//gameServer Ip 생략
	//p->MoveWritePos(16 * sizeof(WCHAR) + sizeof(USHORT));

	p->PutData((char *)GameIP, 16 * sizeof(WCHAR));
	*p << GamePort;
	

	p->PutData((char *)ChatIP, 16*sizeof(WCHAR));
	*p << ChatPort;
	//port
	p->SetDisconnect();
	return p;
}


bool LoginServer::ClientLoginRes_ss(INT64 accountNo, INT64 parameter, int type)
{
	//chat server만 연결하므로 일단 type에 대한 체크는 일단 제외
	//game 서버에 대한 연결이 추가되면 이에대한 처리는 추가되어야함

	//parameter는 현재 sessionID로 사용중
	AcquireSRWLockExclusive(&waiterLock);
	auto iter = _WaiterMap.find(parameter);

	if (iter == _WaiterMap.end())
	{
		ReleaseSRWLockExclusive(&waiterLock);
		return false;
	}

	Waiter *waiter = iter->second;
	_WaiterMap.erase(iter);
	ReleaseSRWLockExclusive(&waiterLock);


	//체크 부분
	//파라미터로 sessionID를 사용
	if (parameter != waiter->SessionID)
	{
		//실패
		Packet *p = MakeLoginResponse(accountNo, dfLOGIN_STATUS_FAIL, NULL, NULL);
		SendPacket(waiter->SessionID, p);
		Packet::Free(p);

		return false;
	}

	Packet *p = MakeLoginResponse(waiter->AccountNo, dfLOGIN_STATUS_OK, waiter->id, waiter->nick);
	

	

	

	SendPacket(waiter->SessionID, p);
	Packet::Free(p);

	waiter->endTime = timeGetTime();

	if (waiter->endTime >= waiter->startTime)
	{
		InterlockedAdd64(&loginAll, waiter->endTime - waiter->startTime);

		if (waiter->endTime - waiter->startTime > loginMax)
		{
			InterlockedExchange(&loginMax, waiter->endTime - waiter->startTime);
		}


		if (waiter->endTime - waiter->startTime < loginMin)
		{
			InterlockedExchange(&loginMin, waiter->endTime - waiter->startTime);
		}
	}
	
	

	_waiterPool->Free(waiter);
	InterlockedIncrement(&successCnt);
	InterlockedIncrement(&successCntTPS);
	

	return true;
}

void LoginServer::ServerDownMsg(BYTE type)
{
	AcquireSRWLockExclusive(&waiterLock);

	for (auto iter = _WaiterMap.begin(); iter != _WaiterMap.end(); iter++)
	{
		Waiter *waiter = iter->second;
		Packet *p = MakeLoginResponse(waiter->AccountNo, dfLOGIN_STATUS_NOSERVER, waiter->id, waiter->nick);


		SendPacket(waiter->SessionID, p);
		Packet::Free(p);
		InterlockedIncrement(&downMsg);
	}

	_WaiterMap.clear();

	ReleaseSRWLockExclusive(&waiterLock);
}