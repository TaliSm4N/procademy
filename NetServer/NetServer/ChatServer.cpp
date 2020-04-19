#include <unordered_map>
#include "NetServerLib.h"
#include "Protocol.h"
#include "ChatServer.h"


ChatServer::ChatServer()
{
	_playerMap = new std::unordered_map<DWORD, st_PLAYER *>;
	_msgQ = new LockFreeQueue<st_UPDATE_MESSAGE>(5000);
	InitializeSRWLock(&_playerMapLock);
}

bool ChatServer::Start(int port, int workerCnt, bool nagle, int maxUser, bool monitoring)
{
	CNetServer::Start(port, workerCnt, nagle, maxUser, monitoring);

	_updateThread = (HANDLE)_beginthreadex(NULL, 0, UpdateThread, this, 0, (unsigned int *)&_updateThreadID);

	return true;
}

bool ChatServer::OnConnectionRequest(WCHAR *ClientIP, int Port)
{
}
void ChatServer::OnRecv(DWORD sessionID, Packet *p)
{
	st_UPDATE_MESSAGE msg;

	p->GetData((char *)&msg.iMsgType, sizeof(WORD));
	msg.SessionID = sessionID;
	msg.pPacket = p;

	_msgQ->Enqueue(msg);
}
void ChatServer::OnSend(DWORD sessionID, int sendsize)
{
	return;
}
void ChatServer::OnClientJoin(DWORD sessionID)
{
	//클라이언트의 프로토콜 약속에 따라서 여기서 연결 완료에 대한 메시지를 쏴줄수도 있음 

	st_PLAYER *player = new st_PLAYER;

	player->SessionID = sessionID;

	//존재할 수 없는 섹터의 좌표
	//해당 섹터의 위치를 가리키는 player는 아직 접속이 완료되지 않음
	player->shSectorX = -1;
	player->shSectorY = -1;
	player->LastRecvPacket = timeGetTime();//최초 시간


	//playerMap에 등록
	AcquireSRWLockExclusive(&_playerMapLock);
	_playerMap->insert(std::make_pair(sessionID, player));
	ReleaseSRWLockExclusive(&_playerMapLock);

}
void ChatServer::OnClientLeave(DWORD sessionID)
{
	//playerMap을 뒤져서 존재하면 해당 플레이어 제거해줌
	//단 player가 없을 경우 그냥 나감

	st_PLAYER *player;
	AcquireSRWLockExclusive(&_playerMapLock);
	auto iter =_playerMap->find(sessionID);

	if (iter == _playerMap->end())
	{
		ReleaseSRWLockExclusive(&_playerMapLock);
		return;
	}
	player = iter->second;
	_playerMap->erase(iter);
	ReleaseSRWLockExclusive(&_playerMapLock);

	//player에 대한 처리를 하고 있었다면 어떻게 처리를 해야하는가
	//제대로 만들때 이부분 고려해서 수정
	delete player;
}
void ChatServer::OnError(int errorcode, WCHAR *)
{
}

unsigned int WINAPI ChatServer::UpdateThread(LPVOID lpParam)
{
	ChatServer *_this = (ChatServer *)lpParam;
	return _this->UpdateThreadRun();
}

unsigned int ChatServer::UpdateThreadRun()
{
	st_UPDATE_MESSAGE msg;
	while (1)
	{
		_msgQ->Dequeue(&msg);

		//msg타입에 따른 동작함수를 넣어주자
		switch (msg.iMsgType)
		{
		case 0:
			break;
		}
	}

	return 0;
}