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
	//SOCKET sock = _ClientInfo.sock;
	_closeSock = _ClientInfo.sock;
	InterlockedExchange(&_ClientInfo.sock, INVALID_SOCKET);
	//_ClientInfo.sock = INVALID_SOCKET;

	if(_closeSock !=INVALID_SOCKET)
		closesocket(_closeSock);
	
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
	//_lSendIO = true;//logout중 send를 막기위해 send 중 상황으로 간주
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

int Session::CompleteRecv(int transferred)
{
	int ret = 0;
	PROCRESULT result;
	if (transferred == 0)
	{
		Disconnect();
		return ret;
	}

	_RecvQ.MoveWritePos(transferred);

	while (1)
	{
		result =CompleteRecvPacket();

		if (result != SUCCESS)
		{
			break;
		}
		ret++;
	}

	if (result != FAIL && !_bLogoutFlag)
		ProcRecv();

	//if (InterlockedDecrement64(&_IOCount) == 0)
	//{
	//	_bLogoutFlag = true;
	//}

	return ret;
}

PROCRESULT Session::CompleteRecvPacket()
{
	int recvQSize = _RecvQ.GetUseSize();
	Packet *payload;
	NetServerHeader header;// = payload->GetHeaderPtr();

	if (sizeof(NetServerHeader) > recvQSize)
	{
		//Packet::Free(payload);
		return NONE;
	}


	_RecvQ.Peek((char *)&header, sizeof(NetServerHeader));


	if (header.len > DEFAULT_PACKET_SIZE)
	{
		return FAIL;
	}

	if (recvQSize < header.len + sizeof(NetServerHeader))
	{
		//Packet::Free(payload);
		return NONE;
	}

	if (header.code != Packet::GetCode())
	{
		return FAIL;
	}

	_RecvQ.MoveReadPos(sizeof(NetServerHeader));

	payload = Packet::Alloc();
	payload->RecvEncode();

	if (_RecvQ.Dequeue(payload, header.len) != header.len)
	{

		Packet::Free(payload);
		return FAIL;
	}

	payload->PutHeader((char *)&header);

	payload->decode();

	//payload 검증부분

	if (!payload->VerifyCheckSum())
	{
		Packet::Free(payload);
		return FAIL;
	}

	//session->CompletePacketQ().push(payload);


	_CompletePacket.Enqueue(payload);
	//Packet::Free(payload);

	return SUCCESS;
}

int Session::CompleteSend(int transferred)
{
	int ret =0;
	if (transferred == 0)
	{
		Disconnect();
		InterlockedExchange(&_lSendIO, false);
		return ret;
	}

	if (_iSendPacketSize != transferred)
	{
		LOG(L"Lib", LOG_WARNING, L"not equal transferred %d : %d %d", iArrayIndex, _iSendPacketSize, transferred);
	}

	for (int i = 0; i <_iSendPacketCnt; i++)
	{
		Packet *temp;
		_SendQ.Dequeue(&temp);
		//temp->FreeTime = GetTickCount();
		//_Monitor_Send_Time += temp->FreeTime - temp->AllocTime;
		Packet::Free(temp);
		ret++;
		//InterlockedIncrement(&_Monitor_PacketSend);
	}
	_iSendPacketCnt = 0;

	//if(!session->GetLogoutFlag())


	InterlockedExchange(&_lSendIO, false);

	//if (InterlockedDecrement64(&_IOCount) == 0)
	//{
	//	_bLogoutFlag = true;
	//}

	return ret;
}

bool Session::ProcSend()
{
	int peekCnt=0;
	int flags = 0;
	Packet *peekData[1024];
	WSABUF wsabuf[1024];

	if (InterlockedCompareExchange(&_lSendIO, true, false))
	{
		return false;
	}

	if (_Mode != MODE_AUTH && _Mode != MODE_GAME)
	{

		InterlockedExchange(&_lSendIO, false);
		return false;
	}

	if (_SendQ.GetUseCount() <= 0)
	{
		InterlockedExchange(&_lSendIO, false);
		return false;
	}

	peekCnt = _SendQ.Peek(peekData, peekCnt);

	if (peekCnt == 0)
	{
		InterlockedExchange(&_lSendIO, false);
		return false;
	}

	_iSendPacketSize = 0;

	for (int i = 0; i < peekCnt; i++)
	{
		wsabuf[i].buf = (char *)peekData[i]->GetSendPtr();
		//wsabuf[i].len = 0;
		wsabuf[i].len = peekData[i]->GetDataSize() + sizeof(NetServerHeader);
		_iSendPacketSize += wsabuf[i].len;
	}

	if (_iSendPacketSize == 0)
	{
		InterlockedExchange(&_lSendIO, false);
		return false;
	}

	_iSendPacketCnt = peekCnt;
	ZeroMemory(&_SendOverlapped, sizeof(OVERLAPPED));
	InterlockedIncrement64(&_IOCount);

	int retval = WSASend(_ClientInfo.sock, wsabuf, peekCnt, NULL, flags,&_SendOverlapped, NULL);


	if (retval == SOCKET_ERROR)
	{
		int err;
		if ((err = WSAGetLastError()) != ERROR_IO_PENDING)
		{
			//PRO_END(L"WSASend");

			//session->Senderr = err;




			if (InterlockedDecrement64(&_IOCount) == 0)
			{
				//SYSLOG_LOG(L"Lib", LOG_WARNING, L"Logout send : %d", session->GetIndex());
				//session->logSend++;
				_bLogoutFlag = true;
				//session->Logout();
				//InterlockedIncrement(&_Monitor_Logout_Counter);

			}

			InterlockedExchange(&_lSendIO, false);


			return false;
		}
	}

	return true;
}

bool Session::ProcRecv()
{
	DWORD flags = 0;
	WSABUF wsabuf[2];

	wsabuf[0].len = _RecvQ.DirectEnqueueSize();
	wsabuf[0].buf = _RecvQ.GetWritePos();
	wsabuf[1].len = _RecvQ.GetFreeSize() - _RecvQ.DirectEnqueueSize();
	wsabuf[1].buf = _RecvQ.GetBufPtr();

	InterlockedIncrement64(&_IOCount);


	ZeroMemory(&_RecvOverlapped, sizeof(OVERLAPPED));
	//PRO_BEGIN(L"WSARecv");
	//session->recvTime = GetTickCount();
	int retval = WSARecv(_ClientInfo.sock, wsabuf, 2, NULL, &flags, (OVERLAPPED *)&_RecvOverlapped, NULL);


	if (retval == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != ERROR_IO_PENDING)
		{
			//PRO_END(L"WSARecv");

			//session->Recverr = err;

			if (InterlockedDecrement64(&_IOCount) == 0)
			{
				//SYSLOG_LOG(L"Lib", LOG_WARNING, L"Logout recv : %d", session->GetIndex());
				//session->logRecv++;
				//session->Logout();
				_bLogoutFlag = true;
				//InterlockedIncrement(&_Monitor_Logout_Counter);
			}

			return false;
		}
	}
	//PRO_END(L"WSARecv");
	//InterlockedIncrement(&_Monitor_RecvOverlap);

	return true;
}