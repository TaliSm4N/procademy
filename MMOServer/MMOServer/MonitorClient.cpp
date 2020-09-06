#include "MonitorProtocol.h"
#include "LanClientLib.h"
#include "MonitorClient.h"


void MonitorClient::OnServerJoin()
{
	Packet *p = MakeServerConnect(dfMONITOR_SERVER_TYPE_GAME);
	_conn = true;

	SendPacket(p);

	Packet::Free(p);
}
void MonitorClient::OnServerLeave()
{
	_conn = false;
}

bool MonitorClient::OnConnectionRequest(WCHAR *ClientIP, int Port)
{
	return true;
}
void MonitorClient::OnRecv(Packet *p)
{
}
void MonitorClient::OnSend(int sendsize)
{}

void MonitorClient::OnError(int errorcode, WCHAR *)
{}

void MonitorClient::SendMonitoringMsg(BYTE	DataType, int DataValue, int TimeStamp)
{

}