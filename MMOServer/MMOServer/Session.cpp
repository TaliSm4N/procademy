#include "MMOLib.h"


void Session::SendPacket(Packet *p)
{
	if (_Mode == MODE_NONE)
		return;
	if (_bLogoutFlag)
		return;

	p->encode();

	p->Ref();
	PRO_BEGIN(L"SEND_ENQ");
	_SendQ.Enqueue(p);
	PRO_END(L"SEND_ENQ");

	return;
}

void Session::CloseSocket()
{
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

	CloseSocket();

	_Mode = MODE_NONE;

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
	_lSendIO = 0;
	_IOCount = 0;
	
	//closesocket(_closeSock);
}

void Session::Disconnect()
{
	CloseSocket();
	//_bLogoutFlag = true;
}

void Session::Logout()
{
	_lSendIO = true;//logout중 send를 막기위해 send 중 상황으로 간주
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