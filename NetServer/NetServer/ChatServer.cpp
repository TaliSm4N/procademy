#include <unordered_map>
#include "NetServerLib.h"
#include "Protocol.h"
#include "ChatServer.h"


ChatServer::ChatServer()
	:_attackDisconCount(0)
{
	_playerMap = new std::unordered_map<DWORD, st_PLAYER *>;
	_msgQ = new LockFreeQueue<st_UPDATE_MESSAGE *>(10000);
}


bool ChatServer::Start(int port, int workerCnt, bool nagle, int maxUser, bool monitoring)
{
	_updateMsgPoolCount = 0;
	_updateMsgQCount = 0;

	_playerCount = 0;
	_playerPoolCount = 0;

	_updateMsgPool = new MemoryPoolTLS<st_UPDATE_MESSAGE>(100);
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
	msg->iMsgType = PACKET;
	msg->SessionID = sessionID;
	msg->pPacket = p;

	p->Ref();
	_msgQ->Enqueue(msg);
	SetEvent(_msgHandle);
}
void ChatServer::OnSend(DWORD sessionID, int sendsize)
{
	return;
}
void ChatServer::OnClientJoin(DWORD sessionID)
{
	st_UPDATE_MESSAGE *msg = _updateMsgPool->Alloc();
	msg->iMsgType = JOIN;
	msg->SessionID = sessionID;

	_msgQ->Enqueue(msg);
	SetEvent(_msgHandle);
}
void ChatServer::OnClientLeave(DWORD sessionID)
{
	st_UPDATE_MESSAGE *msg = _updateMsgPool->Alloc();
	msg->iMsgType = LEAVE;
	msg->SessionID = sessionID;

	_msgQ->Enqueue(msg);
	SetEvent(_msgHandle);
}
void ChatServer::OnError(int errorcode, WCHAR *)
{
}

void ChatServer::Join(DWORD sessionID)
{
	st_PLAYER *player = _playerPool->Alloc();

	player->SessionID = sessionID;

	//������ �� ���� ������ ��ǥ
	//�ش� ������ ��ġ�� ����Ű�� player�� ���� ������ �Ϸ���� ����
	player->shSectorX = -1;
	player->shSectorY = -1;
	player->LastRecvPacket = timeGetTime();//���� �ð�
	player->AccountNo = -1;

	//InterlockedExchange8((char *)&player->connect, true);
	player->login = false;
	//playerMap�� ���
	_playerMap->insert(std::make_pair(sessionID, player));
	_playerCount++;
}

void ChatServer::Leave(DWORD sessionID)
{
	st_PLAYER *player;
	auto iter = _playerMap->find(sessionID);
	

	if (iter == _playerMap->end())
	{
		return;
	}

	player = iter->second;
	player->login = false;
	//InterlockedExchange8((char *)&player->connect, false);

	if (player->shSectorX != -1 && player->shSectorY != -1)
	{
		//���� ���Ϳ��� ������
		_sector[player->shSectorY][player->shSectorX].remove(player);
	}

	_playerMap->erase(iter);

	_playerPool->Free(player);
	
	_playerCount--;
	
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
	st_UPDATE_MESSAGE *msg=NULL;
	
	
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
			if (!_msgQ->Dequeue(&msg))
			{
				volatile int test = 1;
			}

			switch (msg->iMsgType)
			{
			case JOIN:
				Join(msg->SessionID);
				break;
			case LEAVE:
				Leave(msg->SessionID);
				break;
			case PACKET:
				PacketProc(msg);
				break;
			default:
				_attackDisconCount++;
				Disconnect(msg->SessionID);
				break;
			}
			
			_updateMsgPool->Free(msg);
		}

		volatile int test = 1;
	}

	return 0;
}

void ChatServer::PacketProc(st_UPDATE_MESSAGE *msg)
{
	WORD flag = 0;
	//msgŸ�Կ� ���� �����Լ��� �־�����

	*(msg->pPacket) >> flag;

	if (msg->pPacket->GetLastError()!= E_NOERROR)
	{
		_attackDisconCount++;
		Disconnect(msg->SessionID);
		//OnClientLeave(msg->SessionID);
		return;
	}

	switch (flag)
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
	default:
		_attackDisconCount++;
		Disconnect(msg->SessionID);
		break;

	}
	Packet::Free(msg->pPacket);
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

	if (player->login)
	{
		_attackDisconCount++;
		//InterlockedExchange8((char *)&player->connect, false);
		player->login = false;
		Disconnect(sessionID);
	
		return;
	}

	if (player->AccountNo != -1)
	{
		_attackDisconCount++;
		//InterlockedExchange8((char *)&player->connect, false);
		player->login = false;
		Disconnect(sessionID);
		return;
	}


	*p >> player->AccountNo;

	

	p->GetData((char *)player->szID, sizeof(WCHAR) * 20);
	p->GetData((char *)player->szNick, sizeof(WCHAR) * 20);
	p->GetData((char *)player->SessionKey, sizeof(char) * dfSESSION_KEY_BYTE_LEN);
	

	if (p->GetDataSize() > 0)
	{
		_attackDisconCount++;
		player->login = false;
		Disconnect(sessionID);

		return;
	}

	if (p->GetLastError() != E_NOERROR)
	{
		_attackDisconCount++;
		//InterlockedExchange8((char *)&player->connect, false);
		player->login = false;
		Disconnect(sessionID);
		
		return;
	}
	//Packet::Free(p);
	//�ϴ��� ��� ��������
	//���� ����

	player->login = true;

	Packet *sendPacket = MakeResLogin(1,player->AccountNo);
	
	//SendPacket(sessionID, sendPacket);
	SendUnicast(player, sendPacket);

	Packet::Free(sendPacket);
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

	if (p->GetLastError() != E_NOERROR)
	{
		_attackDisconCount++;
		//InterlockedExchange8((char *)&player->connect, false);
		player->login = false;
		Disconnect(sessionID);
		
		return;
	}


	if (player->AccountNo != account)
	{
		_attackDisconCount++;
		//InterlockedExchange8((char *)&player->connect, false);
		player->login = false;
		Disconnect(sessionID);
		
		return;
	}

	//���⼭ ������ player�� ���͸� �Ű�����

	if (player->shSectorX != -1 && player->shSectorY != -1)
	{
		//���� ���Ϳ��� ������

		_sector[player->shSectorY][player->shSectorX].remove(player);
	}
	*p >> player->shSectorX >> player->shSectorY;

	if (p->GetDataSize() > 0)
	{
		_attackDisconCount++;
		player->login = false;
		Disconnect(sessionID);

		return;
	}

	if (p->GetLastError() != E_NOERROR)
	{
		_attackDisconCount++;
		//InterlockedExchange8((char *)&player->connect, false);
		player->login = false;
		Disconnect(sessionID);
		
		return;
	}


	//Packet::Free(p);
	//���ͷ� �ֱ�
	
	_sector[player->shSectorY][player->shSectorX].push_back(player);

	//�Ƹ��� ������ �̵��� update������ �������� �ϱ⶧���� ����ȭ�� ũ�Դ� �ʿ������
	//�� ���� ����ÿ��� ����ȭ�� �ʿ��� ��

	Packet *sendPacket = MakeResMoveSector(player->AccountNo, player->shSectorX, player->shSectorY);

	//SendPacket(sessionID, sendPacket);
	

	SendUnicast(player, sendPacket);
	
	Packet::Free(sendPacket);
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

	if (p->GetLastError() != E_NOERROR)
	{
		_attackDisconCount++;
		//InterlockedExchange8((char *)&player->connect, false);
		player->login = false;
		Disconnect(sessionID);
		
		return;
	}

	if (player->AccountNo != account)
	{
		_attackDisconCount++;
		//InterlockedExchange8((char *)&player->connect, false);
		player->login = false;
		Disconnect(sessionID);
		
		return;
	}

	if (p->GetDataSize() != msgLen)
	{
		_attackDisconCount++;
		//InterlockedExchange8((char *)&player->connect, false);
		player->login = false;
		Disconnect(sessionID);
		
		return;
	}

	//���ο� packet�� copy�ؾ��� �״�� ���� �ȵ�
	WCHAR *msg = (WCHAR *)p->GetBufferPtr();

	

	Packet *sendPacket = MakeResMessage(player, msgLen, msg);

	//Packet::Free(p);

	//sector�� ������ ������ �����ؾ���
	//SendPacket(sessionID, sendPacket);
	SendSectorAround(player->shSectorX, player->shSectorY, sendPacket);
	//SendBroadcast(sendPacket);

	Packet::Free(sendPacket);
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

void ChatServer::SendUnicast(st_PLAYER *player, Packet *p)
{
	//���� �𿡰� send��û ����??

	//if(player->connect)
	if(player->login)
		SendPacket(player->SessionID, p);
}
void ChatServer::SendSector(int x, int y, Packet *p)
{
	st_PLAYER *player;

	for (auto iter = _sector[y][x].begin(); iter != _sector[y][x].end(); iter++)
	{
		player = *iter;

		if(player->shSectorX >= 0 && player->shSectorY >= 0)
			SendUnicast(player, p);
	}
	//������ packet�� ref�� 1�̿��⶧���� �ѹ� ����
}
void ChatServer::SendSectorAround(int x, int y, Packet *p)
{

	//sector�� �������� packet ��ȯ ����

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
}

void ChatServer::SendBroadcast(Packet *p)
{
	for (auto iter = _playerMap->begin(); iter != _playerMap->end(); iter++)
	{
		SendUnicast(iter->second, p);
	}
}