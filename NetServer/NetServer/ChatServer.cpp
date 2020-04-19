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
	//Ŭ���̾�Ʈ�� �������� ��ӿ� ���� ���⼭ ���� �Ϸῡ ���� �޽����� ���ټ��� ���� 

	st_PLAYER *player = new st_PLAYER;

	player->SessionID = sessionID;

	//������ �� ���� ������ ��ǥ
	//�ش� ������ ��ġ�� ����Ű�� player�� ���� ������ �Ϸ���� ����
	player->shSectorX = -1;
	player->shSectorY = -1;
	player->LastRecvPacket = timeGetTime();//���� �ð�


	//playerMap�� ���
	AcquireSRWLockExclusive(&_playerMapLock);
	_playerMap->insert(std::make_pair(sessionID, player));
	ReleaseSRWLockExclusive(&_playerMapLock);

}
void ChatServer::OnClientLeave(DWORD sessionID)
{
	//playerMap�� ������ �����ϸ� �ش� �÷��̾� ��������
	//�� player�� ���� ��� �׳� ����

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

	//player�� ���� ó���� �ϰ� �־��ٸ� ��� ó���� �ؾ��ϴ°�
	//����� ���鶧 �̺κ� ����ؼ� ����
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

		//msgŸ�Կ� ���� �����Լ��� �־�����
		switch (msg.iMsgType)
		{
		case 0:
			break;
		}
	}

	return 0;
}