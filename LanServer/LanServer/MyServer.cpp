#include "LanServerLib.h"
#include "Player.h"
#include "MyServer.h"


CMyServer::CMyServer()
{
}

bool CMyServer::OnConnectionRequest(WCHAR *ClientIP, int Port)
{
	return true;
}

void CMyServer::OnRecv(DWORD sessionID, Packet *p)
{
	Echo(sessionID, p);
}

void CMyServer::OnSend(DWORD sessionID, int sendsize)
{
	return;
}
void CMyServer::OnClientJoin(DWORD sessionID)
{
	//HEADER header;
	Player *player = new Player(sessionID);

	

	//Packet *p = new Packet;
	//Packet *p = PacketAlloc();
	Packet *p = Packet::Alloc();
	//header.len = 8;
	//p->PutHeader(&header);
	//p->PutData((char *)&header, sizeof(header));
	*p << 0x7fffffffffffffff;

	//자동화 테스트
	//PacketPtr *pPtr =new PacketPtr;
	//pPtr->GetPacket()->PutData((char *)&header, sizeof(header));
	//*pPtr << 0x7fffffffffffffff;
	//자동화 테스트

	AcquireSRWLockExclusive(&playerListLock);
	playerList.insert(std::make_pair(sessionID, player));
	SendPacket(sessionID, p);

	//자동화 테스트
	//AutoSendPacket(sessionID, pPtr);
	//자동화 테스트

	ReleaseSRWLockExclusive(&playerListLock);
	
}
void CMyServer::OnClientLeave(DWORD sessionID)
{
	AcquireSRWLockExclusive(&playerListLock);
	auto iter = playerList.find(sessionID);

	if (iter == playerList.end())
	{
		ReleaseSRWLockExclusive(&playerListLock);
		return;
	}
	Player *player = iter->second;
	playerList.erase(playerList.find(sessionID));
	player->Lock();
	ReleaseSRWLockExclusive(&playerListLock);
	player->UnLock();
	delete player;
}
void CMyServer::OnError(int errorcode, WCHAR *)
{}


bool CMyServer::Echo(LONGLONG sessionID, Packet *p)
{
	//echo하기 전에 player에 삭제되면 echo할 이유가 없음
	AcquireSRWLockExclusive(&playerListLock);
	auto iter = playerList.find(sessionID);

	if (iter == playerList.end())
	{
		ReleaseSRWLockExclusive(&playerListLock);
		return FALSE;
	}
	Player *player = iter->second;
	ReleaseSRWLockExclusive(&playerListLock);

	//Packet *sendPacket = PacketAlloc();
	//Packet *sendPacket = new Packet;
	//Packet *sendPacket = Packet::Alloc();
	//LanServerHeader header;

	//LONGLONG data;

	//*p >> data;

	//header.len = sizeof(data);
	//sendPacket->PutData((char *)&header, sizeof(header));
	//*sendPacket << data;

	SendPacket(sessionID, p);


	//자동화 테스트
	//PacketPtr *pPtr = new PacketPtr;
	//pPtr->GetPacket()->PutData((char *)&header, sizeof(header));
	//*pPtr << data;
	//
	//AutoSendPacket(sessionID, pPtr);
	//자동화 테스트

	return TRUE;
}