#include <WinSock2.h>
#include <unordered_map>

#include <strsafe.h>
#pragma comment(lib,"mysqlclient.lib")
#include "include/mysql.h"
#include "include/errmsg.h"

#include "DBConnect.h"
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
	InitializeSRWLock(&dbLock);
}

bool LoginServer::Start(int port, int workerCnt, bool nagle, int maxUser, bool monitoring)
{
	accountDB.connect("127.0.0.1", "root", "1234", "accountdb");

	_connecter.Start(5000, 5, true, 10);

	accountDB.Query("UPDATE `status` SET `status` = 0");

	return CNetServer::Start(port,workerCnt,nagle,maxUser,monitoring);
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
	AcquireSRWLockShared(&waiterLock);
	auto iter = _WaiterMap.find(sessionID);

	if (iter == _WaiterMap.end())
	{
		ReleaseSRWLockShared(&waiterLock);
		return;
	}
	_WaiterMap.erase(iter);
	ReleaseSRWLockShared(&waiterLock);
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

	AcquireSRWLockExclusive(&dbLock);
	
	bool re = accountDB.Query("SELECT sessionkey, userid, usernick, status FROM v_account where accountno = %ld", accountNo);

	DWORD time = accountDB.GetLastQueryTime();

	//5초 이상걸린 쿼리 기록
	if (time > 5000)
	{
		LOG(L"query", LOG_ERROR, L"Query time over %d ms : accountNo = %ld", time, accountNo);
	}

	MYSQL_ROW row = accountDB.GetRow();

	if (row == NULL)
	{
		//실패
		Packet *pk = MakeLoginResponse(accountNo, dfLOGIN_STATUS_FAIL, NULL, NULL);
		SendPacket(sessionID, pk);
		Packet::Free(pk);

		accountDB.FreeResult();

		ReleaseSRWLockExclusive(&dbLock);
		return false;
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

			accountDB.FreeResult();

			ReleaseSRWLockExclusive(&dbLock);
			return false;
		}
	}

	Waiter *waiter = _waiterPool.Alloc();
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
		Packet *pk = MakeLoginResponse(accountNo, dfLOGIN_STATUS_FAIL, NULL, NULL);
		SendPacket(sessionID, pk);
		Packet::Free(pk);

		accountDB.FreeResult();

		ReleaseSRWLockExclusive(&dbLock);
		return false;
	}

	accountDB.FreeResult();
	ReleaseSRWLockExclusive(&dbLock);

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
	p->MoveWritePos(16 * sizeof(WCHAR) + sizeof(USHORT));
	
	WCHAR ip[16] = L"127.0.0.1";
	p->PutData((char *)ip, 16*sizeof(WCHAR));
	*p << (USHORT)6000;
	//port

	return p;
}


bool LoginServer::ClientLoginRes_ss(INT64 accountNo, INT64 parameter, int type)
{
	//chat server만 연결하므로 일단 type에 대한 체크는 일단 제외
	//game 서버에 대한 연결이 추가되면 이에대한 처리는 추가되어야함

	//parameter는 현재 sessionID로 사용중
	AcquireSRWLockShared(&waiterLock);
	auto iter = _WaiterMap.find(parameter);

	if (iter == _WaiterMap.end())
	{
		ReleaseSRWLockShared(&waiterLock);
		return false;
	}

	Waiter *waiter = iter->second;
	_WaiterMap.erase(iter);
	ReleaseSRWLockShared(&waiterLock);

	Packet *p = MakeLoginResponse(waiter->AccountNo, dfLOGIN_STATUS_OK, waiter->id, waiter->nick);


	//체크 부분
	//

	SendPacket(waiter->SessionID, p);

	waiter->endTime = timeGetTime();

	InterlockedAdd64(&loginAll, waiter->endTime - waiter->startTime);

	if (waiter->endTime - waiter->startTime > loginMax)
	{
		InterlockedExchange(&loginMax, waiter->endTime - waiter->startTime);
	}

	if (waiter->endTime - waiter->startTime < loginMin)
	{
		InterlockedExchange(&loginMin, waiter->endTime - waiter->startTime);
	}

	_waiterPool.Free(waiter);
	Packet::Free(p);
	InterlockedIncrement(&successCnt);
	

	return true;
}