#include "LanClientLib.h"
#include "Player.h"
#include "MyClient.h"


CMyClient::CMyClient()
{
}

bool CMyClient::OnConnectionRequest(WCHAR *ClientIP, int Port)
{
	return true;
}

void CMyClient::OnRecv(Packet *p)
{
	Echo(p);
}

void CMyClient::OnSend(int sendsize)
{
	return;
}
void CMyClient::OnServerJoin()
{
}
void CMyClient::OnServerLeave()
{

}
void CMyClient::OnError(int errorcode, WCHAR *)
{}


bool CMyClient::Echo(Packet *p)
{
	//echo�ϱ� ���� player�� �����Ǹ� echo�� ������ ����



	SendPacket(p);


	return TRUE;
}