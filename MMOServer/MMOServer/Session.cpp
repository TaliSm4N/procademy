#include "MMOLib.h"


void Session::SendPacket(Packet *p)
{
	if (_Mode == MODE_NONE)
		return;
	if (_bLogoutFlag)
		return;

	p->encode();

	p->Ref();
	//PRO_BEGIN(L"SEND_ENQ");
	_SendQ.Enqueue(p);
	//PRO_END(L"SEND_ENQ");

	return;
}

void Session::CloseSocket()
{
	//SYSLOG_LOG(L"Lib", LOG_WARNING, L"close socket pos : %d %d", iArrayIndex, _iSendPacketSize);
	SOCKET sock = _ClientInfo.sock;
	_closeSock = _ClientInfo.sock;
	InterlockedExchange(&_ClientInfo.sock, INVALID_SOCKET);
	//_ClientInfo.sock = INVALID_SOCKET;

	if(sock!=INVALID_SOCKET)
		closesocket(sock);
	
}

void Session::Reset()
{
	Packet *temp;


	_Mode = MODE_NONE;

	CloseSocket();
	//closesocket(_ClientInfo.sock);

	_RecvQ.Reset();

	while (_SendQ.GetUseCount() > 0)
	{
		_SendQ.Dequeue(&temp);

		Packet::Free(temp);
	}

	while (_CompletePacket.GetUseCount() > 0)
	{
		_CompletePacket.Dequeue(&temp);
	
		Packet::Free(temp);
	}

	//while (!_CompletePacket.empty())
	//{
	//	temp = _CompletePacket.front();
	//	_CompletePacket.pop();
	//
	//	Packet::Free(temp);
	//}
	

	ZeroMemory(&_RecvOverlapped, sizeof(OVERLAPPED));
	ZeroMemory(&_SendOverlapped, sizeof(OVERLAPPED));
	_bLogoutFlag = false;
	_bAuthToGameFlag = false;
	_iSendPacketCnt = 0;
	_iSendPacketSize = 0;
	//_lSendIO = 0;
	InterlockedExchange(&_lSendIO, false);
	//_IOCount = 0;


	//logSend = 0;
	//logIOCP = 0;
	//logRecv = 0;
	//logAccept = 0;
	//
	//Senderr = 0;
	//Recverr = 0;
	
	//closesocket(_closeSock);
}

void Session::Disconnect()
{
	//_bLogoutFlag = true;
	//shutdown(_ClientInfo.sock, SD_BOTH);
	CloseSocket();
}

void Session::Logout()
{
	//SYSLOG_LOG(L"Lib", LOG_WARNING, L"llogout : %d - %d %d", iArrayIndex,Recverr,Senderr);
	//_lSendIO = true;//logout�� send�� �������� send �� ��Ȳ���� ����
	_bLogoutFlag = true;
}

void Session::OnAuth_ClientJoin()
{
}
void Session::OnAuth_ClientLeave()
{
}
void Session::OnAuth_Packet(Packet *p)
{
}

void Session::OnGame_ClientJoin()
{
}
void Session::OnGame_ClientLeave()
{
}
void Session::OnGame_ClientRelease()
{
	
}
void Session::OnGame_Packet(Packet *p)
{
}

bool Session::ValidMode(en_SESSION_MODE mode)
{
	bool ret = false;

	if (_Mode != mode)
		return ret;

	if (!(_bLogoutFlag || _bAuthToGameFlag))
		ret = true;

	return ret;
}