#include <unordered_map>

#include "CommonProtocol.h"
#include "LanClientLib.h"
#include "NetServerLib.h"
#include "Connector.h"
#include "ChatServer.h"

Connector::Connector()
{}

void Connector::Init(ChatServer *chat)
{
	_chatServer = chat;
}


void Connector::OnServerJoin()
{
	Packet *p = MakeServerConnect(dfSERVER_TYPE_CHAT);

	SendPacket(p);

	Packet::Free(p);
}
void Connector::OnServerLeave()
{}

bool Connector::OnConnectionRequest(WCHAR *ClientIP, int Port)
{
	return true;
}
void Connector::OnRecv(Packet *p)
{
	WORD type;

	*p >> type;

	switch (type)
	{
	case en_PACKET_SS_REQ_NEW_CLIENT_LOGIN:
		ClientLogin(p);
		break;
	}
}
void Connector::OnSend(int sendsize)
{}

void Connector::OnError(int errorcode, WCHAR *)
{}

bool Connector::ClientLogin(Packet *p)
{
	INT64 accountNo;
	char sessionKey[64];
	INT64 parameter;

	*p >> accountNo;
	p->GetData(sessionKey, 64 * sizeof(char));
	*p >> parameter;

	//chatserver에 세션키 정보 전달파트 추가

	_chatServer->AddNewKey(accountNo, parameter, sessionKey);


	Packet *pk = MakeClientLoginRes(accountNo, parameter);

	SendPacket(pk);

	Packet::Free(pk);

	return true;
}

Packet *Connector::MakeServerConnect(BYTE type)
{
	Packet *p = Packet::Alloc(LOCAL_TYPE);
	*p << (WORD)en_PACKET_SS_LOGINSERVER_LOGIN << type;

	return p;
}

Packet *Connector::MakeClientLoginRes(INT64 accountNo, INT64 parameter)
{
	Packet *p = Packet::Alloc(LOCAL_TYPE);

	*p << (WORD)en_PACKET_SS_RES_NEW_CLIENT_LOGIN << accountNo << parameter;

	return p;
}