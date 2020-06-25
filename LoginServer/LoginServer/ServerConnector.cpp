#include <WinSock2.h>
#include <unordered_map>
#pragma comment(lib,"mysqlclient.lib")
#include "include/mysql.h"
#include "include/errmsg.h"

#include "DBConnect.h"
#include "CommonProtocol.h"
#include "LanServerLib.h"
#include "NetServerLib.h"
#include "ServerConnector.h"
#include "loginServer.h"



ServerConnector::ServerConnector()
{
	InitializeSRWLock(&_serverMapLock);
	
}

void ServerConnector::Init(LoginServer *login)
{
	_loginServer = login;
}

bool ServerConnector::OnConnectionRequest(WCHAR *ClientIP, int Port)
{
	//내부 server white ip를 등록해서 그것만 통과되도록 설정?

	return true;
}
void ServerConnector::OnRecv(DWORD sessionID, Packet *p)
{
	WORD type;

	*p >> type;

	switch (type)
	{
	case en_PACKET_SS_LOGINSERVER_LOGIN:
		ServerLogin(sessionID, p);
		break;
	case en_PACKET_SS_RES_NEW_CLIENT_LOGIN:
		ClientLoginRes(sessionID, p);
		break;
	}
}

void ServerConnector::OnSend(DWORD sessionID, int sendsize)
{}

void ServerConnector::OnClientJoin(DWORD sessionID)
{
	ConnectedServer *server = new ConnectedServer;
	server->sessionID = sessionID;

	AcquireSRWLockExclusive(&_serverMapLock);
	_serverMap.insert(std::make_pair(sessionID, server));
	ReleaseSRWLockExclusive(&_serverMapLock);
}
void ServerConnector::OnClientLeave(DWORD sessionID)
{
	AcquireSRWLockExclusive(&_serverMapLock);

	auto iter = _serverMap.find(sessionID);

	if (iter != _serverMap.end())
	{
		_loginServer->ServerDownMsg(iter->second->type);
	}

	_serverMap.erase(sessionID);
	ReleaseSRWLockExclusive(&_serverMapLock);
}
void ServerConnector::OnError(int errorcode, WCHAR *)
{}

bool ServerConnector::ServerLogin(DWORD sessionID, Packet *p)
{
	AcquireSRWLockShared(&_serverMapLock);
 	ConnectedServer *server = _serverMap.find(sessionID)->second;
	*p >> server->type;
	//p->GetData((char *)server->IP, 16*sizeof(WCHAR));
	//*p >> server->port;
	
	//switch (type)
	//{
	//case dfSERVER_TYPE_GAME:
	//	break;
	//case dfSERVER_TYPE_CHAT:
	//	break;
	//case dfSERVER_TYPE_MONITOR:
	//	break;
	//}

	ReleaseSRWLockShared(&_serverMapLock);

	return true;
}

bool ServerConnector::ClientLoginRes(DWORD sessionID, Packet *p)
{
	INT64 accountNo;
	INT64 parameter;
	*p >> accountNo >> parameter;
	AcquireSRWLockShared(&_serverMapLock);
	ConnectedServer *server = _serverMap.find(sessionID)->second;

	_loginServer->ClientLoginRes_ss(accountNo, parameter, server->type);
	ReleaseSRWLockShared(&_serverMapLock);




	return true;
}

bool ServerConnector::ClientLoginReq_ss(INT64 accountNo,char *sessionKey, INT64 parameter, int type)
{
	ConnectedServer *server = GetServer(type);

	if (server == NULL)
		return false;

	Packet *p = Packet::Alloc(LOCAL_TYPE);

	*p << (WORD)en_PACKET_SS_REQ_NEW_CLIENT_LOGIN << accountNo;
	p->PutData(sessionKey, 64 * sizeof(char));
	*p << parameter;

	SendPacket(server->sessionID, p);
	Packet::Free(p);

	return true;
}

ServerConnector::ConnectedServer *ServerConnector::GetServer(int type)
{
	AcquireSRWLockShared(&_serverMapLock);
	for (auto iter = _serverMap.begin(); iter != _serverMap.end(); iter++)
	{
		if (iter->second->type == type)
		{
			ReleaseSRWLockShared(&_serverMapLock);
			return iter->second;
		}
	}
	ReleaseSRWLockShared(&_serverMapLock);
	return NULL;
}