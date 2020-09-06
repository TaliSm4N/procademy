#include <vector>
#include "MonitorProtocol.h"
#include "NetServerLib.h"
#include "ClientConnector.h"

ClientConnector::ClientConnector()
{
}

bool ClientConnector::Start(int port, int workerCnt, bool nagle, int maxUser, bool monitoring)
{
	return CNetServer::Start(port, workerCnt, nagle, maxUser, monitoring);
}

bool ClientConnector::Start()
{
	return CNetServer::Start();
}

bool ClientConnector::Config(const WCHAR *configFile)
{
	if (!CNetServer::Config(configFile, L"SERVER"))
		return false;
	TextParser parser;
	std::wstring str;

	if (!parser.init(configFile))
		return false;

	if (!parser.SetCurBlock(L"MonitorServer"))
		return false;

	return true;
}

void ClientConnector::OnClientJoin(DWORD sessionID)
{
}
void ClientConnector::OnClientLeave(DWORD sessionID)
{
}
bool ClientConnector::OnConnectionRequest(WCHAR *ClientIP, int Port)
{
}
void ClientConnector::OnRecv(DWORD sessionID, Packet *p)
{
	WORD type;
	*p >> type;

	switch (type)
	{
	case en_PACKET_CS_MONITOR_TOOL_REQ_LOGIN:
		ClientLogin(sessionID, p);
		break;
	}
}

void ClientConnector::OnSend(DWORD sessionID, int sendsize)
{
}

void ClientConnector::OnError(int errorcode, WCHAR *)
{
}

void ClientConnector::ClientLogin(DWORD sessionID, Packet *p)
{
	char LoginKey[32];

	p->GetData(LoginKey, 32);

	//key °Ë»ç

	Packet *pk = MakeLogin(1);

	Monitor *mon = new Monitor;
	mon->sessionID = sessionID;
	AcquireSRWLockExclusive(&monitorLock);

	monitorVec.push_back(mon);

	ReleaseSRWLockExclusive(&monitorLock);

	SendPacket(sessionID, pk);
	Packet::Free(pk);

	return;
}

Packet *ClientConnector::MakeLogin(BYTE stat)
{
	Packet *p = Packet::Alloc();

	*p << (WORD)en_PACKET_CS_MONITOR_TOOL_RES_LOGIN << stat;

	return p;
}

void ClientConnector::SendMonitoring(BYTE ServerNo, BYTE DataType, int DataValue, int TimeStamp)
{
	Packet *p = Packet::Alloc();

	*p >> ServerNo >> DataType >> DataValue >> TimeStamp;

	AcquireSRWLockExclusive(&monitorLock);
	for (auto iter = monitorVec.begin(); iter != monitorVec.end(); iter++)
	{
		SendPacket((*iter)->sessionID, p);
	}
	ReleaseSRWLockExclusive(&monitorLock);
	Packet::Free(p);
}