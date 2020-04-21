#include <unordered_map>
#include "NetServerLib.h"
#include "Protocol.h"
#include "ChatServer.h"


ChatServer::ChatServer()
{
	_playerMap = new std::unordered_map<DWORD, st_PLAYER *>;
	_msgQ = new LockFreeQueue<st_UPDATE_MESSAGE *>(5000);
	InitializeSRWLock(&_playerMapLock);
}



bool ChatServer::Start(int port, int workerCnt, bool nagle, int maxUser, bool monitoring)
{
	_updateMsgPoolCount = 0;
	_updateMsgQCount = 0;

	_playerCount = 0;
	_playerPoolCount = 0;

	_updateMsgPool = new MemoryPoolTLS<st_UPDATE_MESSAGE>(20);
	_playerPool = new MemoryPoolTLS<st_PLAYER>(maxUser/200+1);
	CNetServer::Start(port, workerCnt, nagle, maxUser, monitoring);

	_msgHandle = CreateEvent(NULL, FALSE, FALSE, NULL);

	_updateThread = (HANDLE)_beginthreadex(NULL, 0, UpdateThread, this, 0, (unsigned int *)&_updateThreadID);

	return true;
}

bool ChatServer::ConfigStart(const WCHAR *configFile)
{
	_updateMsgPoolCount = 0;
	_updateMsgQCount = 0;

	_playerCount = 0;
	_playerPoolCount = 0;

	_updateMsgPool = new MemoryPoolTLS<st_UPDATE_MESSAGE>(20);
	_playerPool = new MemoryPoolTLS<st_PLAYER>(GetMaxUser() / 200 + 1);
	CNetServer::ConfigStart(configFile);

	_msgHandle = CreateEvent(NULL, FALSE, FALSE, NULL);

	_updateThread = (HANDLE)_beginthreadex(NULL, 0, Monitor, this, 0, (unsigned int *)&_monitorID);
	_updateThread = (HANDLE)_beginthreadex(NULL, 0, UpdateThread, this, 0, (unsigned int *)&_updateThreadID);

	return true;
}

bool ChatServer::OnConnectionRequest(WCHAR *ClientIP, int Port)
{
	return true;
}
void ChatServer::OnRecv(DWORD sessionID, Packet *p)
{
	st_UPDATE_MESSAGE *msg=_updateMsgPool->Alloc();

	p->GetData((char *)&msg->iMsgType, sizeof(WORD));
	msg->SessionID = sessionID;
	msg->pPacket = p;

	_msgQ->Enqueue(msg);
	SetEvent(_msgHandle);
}
void ChatServer::OnSend(DWORD sessionID, int sendsize)
{
	return;
}
void ChatServer::OnClientJoin(DWORD sessionID)
{
	//Ŭ���̾�Ʈ�� �������� ��ӿ� ���� ���⼭ ���� �Ϸῡ ���� �޽����� ���ټ��� ���� 

	st_PLAYER *player = _playerPool->Alloc();

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

	InterlockedIncrement((LONG *)&_playerCount);

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
	_playerPool->Free(player);
}
void ChatServer::OnError(int errorcode, WCHAR *)
{
}

unsigned int WINAPI ChatServer::UpdateThread(LPVOID lpParam)
{
	ChatServer *_this = (ChatServer *)lpParam;
	return _this->UpdateThreadRun();
}

unsigned int WINAPI ChatServer::Monitor(LPVOID lpParam)
{
	ChatServer *_this = (ChatServer *)lpParam;
	return _this->MonitorThreadRun();
}

unsigned int ChatServer::UpdateThreadRun()
{
	st_UPDATE_MESSAGE *msg;
	while (1)
	{
		int ret = WaitForSingleObject(_msgHandle, INFINITE);

		if (ret != WAIT_OBJECT_0)
		{
			CrashDump::Crash();
			return -1;
		}

		while (_msgQ->GetUseCount() != 0)
		{
			_msgQ->Dequeue(&msg);

			//msgŸ�Կ� ���� �����Լ��� �־�����
			switch (msg->iMsgType)
			{
			case en_PACKET_CS_CHAT_REQ_LOGIN:
				ReqLogin(msg->SessionID, msg->pPacket);
				break;
			case en_PACKET_CS_CHAT_REQ_SECTOR_MOVE:
				ReqSectorMove(msg->SessionID, msg->pPacket);
				break;
			case en_PACKET_CS_CHAT_REQ_MESSAGE:
				ReqMessage(msg->SessionID, msg->pPacket);
				break;

			}
			_updateMsgPool->Free(msg);
		}

		
	}

	return 0;
}

unsigned int ChatServer::MonitorThreadRun()
{
	while (1)
	{
		_updateMsgPoolCount = _updateMsgPool->GetCount();
		_updateMsgQCount = _msgQ->GetUseCount();
		_playerPoolCount = _playerPool->GetCount();
		//_playerCount=_playerMap->size();
		Sleep(1000);
	}
}

void ChatServer::ReqLogin(DWORD sessionID, Packet *p)
{
	//����ȭ�� ���� Ȯ���� �ȼ� �׽�Ʈ�� �ʿ�� ����ȭ
	auto iter = _playerMap->find(sessionID);

	if (iter == _playerMap->end())
	{
		return;
	}

	st_PLAYER *player = iter->second;

	*p >> player->AccountNo;
	p->GetData((char *)player->szID, sizeof(WCHAR) * 20);
	p->GetData((char *)player->szNick, sizeof(WCHAR) * 20);
	p->GetData((char *)player->SessionKey, sizeof(char) * dfSESSION_KEY_BYTE_LEN);

	Packet::Free(p);
	//�ϴ��� ��� ��������
	//���� ����
	Packet *sendPacket = MakeResLogin(1,player->AccountNo);
	
	SendPacket(sessionID, sendPacket);
}

void ChatServer::ReqSectorMove(DWORD sessionID, Packet *p)
{
	//����ȭ�� ���� Ȯ���� �ȼ� �׽�Ʈ�� �ʿ�� ����ȭ
	auto iter = _playerMap->find(sessionID);

	if (iter == _playerMap->end())
	{
		return;
	}

	st_PLAYER *player = iter->second;
	INT64 account;
	*p >> account;
	if (player->AccountNo != account)
	{
		CrashDump::Crash();
		return;
	}

	//���⼭ ������ player�� ���͸� �Ű�����

	if (player->shSectorX != -1 && player->shSectorY != -1)
	{
		//���� ���Ϳ��� ������

		_sector[player->shSectorY][player->shSectorX].remove(player);
	}
	*p >> player->shSectorX >> player->shSectorY;


	Packet::Free(p);
	//���ͷ� �ֱ�
	
	_sector[player->shSectorY][player->shSectorX].push_back(player);

	//�Ƹ��� ������ �̵��� update������ �������� �ϱ⶧���� ����ȭ�� ũ�Դ� �ʿ������
	//�� ���� ����ÿ��� ����ȭ�� �ʿ��� ��

	Packet *sendPacket = MakeResMoveSector(player->AccountNo, player->shSectorX, player->shSectorY);

	SendPacket(sessionID, sendPacket);
	
}

void ChatServer::ReqMessage(DWORD sessionID, Packet *p)
{
	//����ȭ�� ���� Ȯ���� �ȼ� �׽�Ʈ�� �ʿ�� ����ȭ
	auto iter = _playerMap->find(sessionID);

	if (iter == _playerMap->end())
	{
		return;
	}

	st_PLAYER *player = iter->second;
	INT64 account;
	WORD msgLen;

	*p >> account>>msgLen;
	if (player->AccountNo != account)
	{
		CrashDump::Crash();
		return;
	}

	//���ο� packet�� copy�ؾ��� �״�� ���� �ȵ�
	WCHAR *msg = (WCHAR *)p->GetBufferPtr();

	Packet *sendPacket = MakeResMessage(player, msgLen, msg);

	Packet::Free(p);

	//sector�� ������ ������ �����ؾ���
	SendPacket(sessionID, sendPacket);

	//SendSector(player->shSectorX, player->shSectorY, p);
}

Packet *ChatServer::MakeResLogin(BYTE status,INT64 accountID)
{
	Packet *p = Packet::Alloc();

	if (p == NULL)
		return NULL;

	*p << (WORD)en_PACKET_CS_CHAT_RES_LOGIN <<status << accountID;

	return p;
}

Packet *ChatServer::MakeResMoveSector(INT64 accountID, WORD x, WORD y)
{
	Packet *p = Packet::Alloc();

	if (p == NULL)
		return NULL;

	*p << (WORD)en_PACKET_CS_CHAT_RES_SECTOR_MOVE << accountID << x << y;

	return p;
}

Packet *ChatServer::MakeResMessage(st_PLAYER *player, WORD msgLen, WCHAR *msg)
{
	Packet *p = Packet::Alloc();

	if (p == NULL)
		return NULL;

	*p << (WORD)en_PACKET_CS_CHAT_RES_MESSAGE << player->AccountNo;
	p->PutData((char *)player->szID, sizeof(WCHAR) * 20);
	p->PutData((char *)player->szNick, sizeof(WCHAR) * 20);
	*p << msgLen;
	p->PutData((char *)msg, sizeof(WCHAR)*msgLen);

	return p;
}

void ChatServer::SendUnicast(DWORD sessionID, Packet *p)
{
	SendPacket(sessionID, p);
}
void ChatServer::SendSector(int x, int y, Packet *p)
{
	st_PLAYER *player;
	for (auto iter = _sector[y][x].begin(); iter != _sector[y][x].end(); iter++)
	{
		p->Ref();
		player = *iter;


		SendPacket(player->SessionID, p);
	}
	//������ packet�� ref�� 1�̿��⶧���� �ѹ� ����
	Packet::Free(p);
}
void ChatServer::SendSectorAround(int x, int y, Packet *p)
{

	//sector�� �������� packet ��ȯ ����
	p->Ref();
	for (int i = -1; i <= 1; i++)
	{
		if (x + i >= 0 && x + i < dfSECTOR_MAX)
		{
			for (int j = -1; j <= 1; j++)
			{
				if (y + j >= 0 && y + j < dfSECTOR_MAX)
				{
					SendSector(x + i, y + j, p);
				}
			}
		}
	}

	
	Packet::Free(p);
}