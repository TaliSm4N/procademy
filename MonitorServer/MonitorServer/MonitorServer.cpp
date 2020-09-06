#include <WinSock2.h>
#include <unordered_map>

#include <strsafe.h>
#include <string>
#include <cstring>
#include <list>
#pragma comment(lib,"mysqlclient.lib")
#include "include/mysql.h"
#include "include/errmsg.h"

#include "DBConnect.h"
#include "DBConnectTLS.h"
#include "MonitorProtocol.h"

#include "LanServerLib.h"
#include "NetServerLib.h"
#include "ClientConnector.h"
#include "MonitorServer.h"

MonitorServer::MonitorServer()
{
	InitializeSRWLock(&_serverLock);
}

bool MonitorServer::Start(int port, int workerCnt, bool nagle, int maxUser, bool monitoring)
{
	LogDB.init("127.0.0.1", "root", "1234", "logdb");

	return CLanServer::Start(port, workerCnt, nagle, maxUser, monitoring);
}

bool MonitorServer::Start()
{

	return CLanServer::Start();
}

bool MonitorServer::Config(const WCHAR *configFile)
{
	if (!CLanServer::Config(configFile, L"SERVER"))
		return false;
	TextParser parser;
	std::wstring str;

	if (!parser.init(configFile))
		return false;

	if (!parser.SetCurBlock(L"MonitorServer"))
		return false;

	return true;
}

bool MonitorServer::OnConnectionRequest(WCHAR *ClientIP, int Port)
{
	return true;
}
void MonitorServer::OnRecv(DWORD sessionID, Packet *p)
{
	WORD type;
	*p >> type;

	switch (type)
	{
	case en_PACKET_SS_MONITOR_LOGIN:
		MonitorLogin(sessionID, p);
		break;
	case en_PACKET_SS_MONITOR_DATA_UPDATE:
		MonitorDataUpdate(sessionID, p);
		break;
	}
}

void MonitorServer::OnSend(DWORD sessionID, int sendsize)
{}
void MonitorServer::OnClientJoin(DWORD sessionID)
{
	Server *server = new Server;
	server->id = sessionID;
	AcquireSRWLockExclusive(&_serverLock);
	_serverMap.insert(std::make_pair(sessionID, server));
	ReleaseSRWLockExclusive(&_serverLock);
}
void MonitorServer::OnClientLeave(DWORD sessionID)
{
	AcquireSRWLockExclusive(&_serverLock);

	auto iter = _serverMap.find(sessionID);

	delete iter->second;

	_serverMap.erase(sessionID);
	ReleaseSRWLockExclusive(&_serverLock);

}
void MonitorServer::OnError(int errorcode, WCHAR *)
{}

void MonitorServer::MonitorLogin(DWORD sessionID, Packet *p)
{
	int serverNo;
	*p >> serverNo;

	AcquireSRWLockShared(&_serverLock);
	auto iter = _serverMap.find(sessionID);

	if (iter != _serverMap.end())
	{
		Server *server = iter->second;
		server->serverNo = serverNo;
	}
	ReleaseSRWLockShared(&_serverLock);
}

void MonitorServer::MonitorDataUpdate(DWORD sessionID, Packet *p)
{
	BYTE dataType;
	int dataValue;
	int timeStamp;

	*p >> dataType >> dataValue >> timeStamp;

	AcquireSRWLockShared(&_serverLock);
	auto iter = _serverMap.find(sessionID);

	if (iter != _serverMap.end())
	{
		Server *server = iter->second;
		DBConnect *db = LogDB.GetDB();

		bool re = db->Query("INSERT INTO monitorlog (logtime,serverno,servername,type,value) VALUES (%d,%d,test,%d,%d)", timeStamp, server->serverNo, dataType, dataValue);

		if (!re)
		{
			volatile int test = 1;
		}

		//net으로 전송 파트 추가

		_connector.SendMonitoring(server->serverNo, dataType, dataValue, timeStamp);
	}
	ReleaseSRWLockShared(&_serverLock);
}