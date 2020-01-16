#pragma comment(lib,"ws2_32")
#include <WinSock2.h>
#include <map>
#include <list>

#include "Packet.h"
#include "RingBuffer.h"
#include "Log.h"
#include "network.h"
#include "Game.h"
#include "Protocol.h"
#include "PacketProc.h"
#include "MakePacket.h"
#include "Sector.h"
#include "monitor.h"

SOCKET g_listenSock;

std::map<DWORD, Session *> g_sessionMap;

//listen socket setting
bool InitNetwork()
{

	SOCKADDR_IN addr;
	ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(dfNETWORK_PORT);

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return false;

	g_listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (g_listenSock == INVALID_SOCKET)
	{
		return false;
	}

	if (bind(g_listenSock, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		return false;
	}
	

	if (listen(g_listenSock, SOMAXCONN) == SOCKET_ERROR)
	{
		return false;
	}

	u_long on = 1;

	if (ioctlsocket(g_listenSock, FIONBIO, &on) == SOCKET_ERROR)
	{
		return false;
	}

	return true;
}

bool NetworkProcess()
{
	DWORD SessionIDTable[FD_SETSIZE];
	SOCKET SessionSockTable[FD_SETSIZE];
	int sockCount = 0;

	FD_SET ReadSet;
	FD_SET WriteSet;

	Session *session;

	monitorUnit.MonitorNetwork(START);

	FD_ZERO(&ReadSet);
	FD_ZERO(&WriteSet);
	memset(SessionIDTable, 0, sizeof(DWORD) * FD_SETSIZE);
	memset(SessionSockTable, INVALID_SOCKET, sizeof(SOCKET) * FD_SETSIZE);

	//listen socket 등록
	FD_SET(g_listenSock, &ReadSet);

	SessionIDTable[sockCount] = 0;
	SessionSockTable[sockCount] = g_listenSock;
	sockCount++;
	
	for (auto iter = g_sessionMap.begin(); iter != g_sessionMap.end();)
	{
		session = iter->second;
		iter++;

		SessionIDTable[sockCount] = session->sessionID;
		SessionSockTable[sockCount] = session->sock;

		//FD set 등록
		FD_SET(session->sock, &ReadSet);

		if (session->SendQ.GetUseSize() > 0)
			FD_SET(session->sock, &WriteSet);
		sockCount++;
		
		//select 가득 찼을 경우 select 호출
		if (FD_SETSIZE <= sockCount)
		{
			//select
			CallSelect(SessionIDTable, SessionSockTable, &ReadSet, &WriteSet,sockCount);
			
			FD_ZERO(&ReadSet);
			FD_ZERO(&WriteSet);
			memset(SessionIDTable, 0, sizeof(DWORD) * FD_SETSIZE);
			memset(SessionSockTable, INVALID_SOCKET, sizeof(SOCKET) * FD_SETSIZE);
			sockCount = 0;
		}
	}

	if(sockCount>0)
		CallSelect(SessionIDTable, SessionSockTable, &ReadSet, &WriteSet, sockCount);
	
	monitorUnit.MonitorNetwork(END);
	return true;
}

bool CallSelect(DWORD *IDTable, SOCKET *sockTable, FD_SET *ReadSet, FD_SET *WriteSet,int cnt)
{
	timeval Time;
	int result;

	Time.tv_sec = 0;
	Time.tv_usec = 0;

	result = select(0, ReadSet, WriteSet, NULL, &Time);

	//select 감지 시
	if (result > 0)
	{
		for (int i = 0; i < cnt; i++)
		{
			if (sockTable[i] == INVALID_SOCKET)
				continue;

			//write 체크
			if (FD_ISSET(sockTable[i], WriteSet))
			{
				//send
				ProcSend(IDTable[i]);
			}

			//read 체크
			if (FD_ISSET(sockTable[i], ReadSet))
			{
				if (sockTable[i] == g_listenSock)
				{
					//리슨 소켓 반응
					//연결 요청
					ProcAccept();
				}
				else
				{
					//recv
					if (!ProcRecv(IDTable[i]))
					{
						//disconnect
						DisconnectSession(g_sessionMap.find(IDTable[i])->second);
					}
				}
			}
		}
	}
	else if (result == SOCKET_ERROR)
	{
		//오류 발생
		return false;
	}

	return true;
}

bool ProcAccept()
{
	Session *session;
	SOCKET sock;
	SOCKADDR_IN addr;
	int addrLen = sizeof(addr);
	static DWORD sessionID = 1;

	
	//접속 요청이 올 경우 한번에 모두 처리
	while (1)
	{
		
		sock = accept(g_listenSock, (sockaddr *)&addr, &addrLen);
		
		//accept 할 수 없을 경우 while을 빠져나감
		if (sock == INVALID_SOCKET)
		{
			break;
		}

		_LOG(dfLOG_LEVEL_DEBUG, L"ProcAccept: sessionID(%d)\n", sessionID);
		session = new Session();
		session->sock = sock;
		session->sessionID = sessionID;
		sessionID++;
		session->recvPacketCount = 0;
		session->recvTime = timeGetTime();

		//세션 추가
		g_sessionMap.insert(std::make_pair(session->sessionID, session));
		ConnectSession(session);
	}

	return true;
}

bool ProcRecv(DWORD sID)
{
	int recvSize;
	PROCRESULT result;
	Session *session = g_sessionMap.find(sID)->second;

	//없는 세션
	if (session == NULL)
		return false;

	recvSize = recv(session->sock, session->RecvQ.GetWritePos(), session->RecvQ.DirectEnqueueSize(), 0);
	monitorUnit.MonitorRecv();
	//_LOG(dfLOG_LEVEL_DEBUG, L"ProcRecv");
	//_LOG(dfLOG_LEVEL_DEBUG, L"SessionID: %d\n",sID);
	//socket 에러 또는 recvQ가 가득찼을 경우
	if (recvSize == SOCKET_ERROR || recvSize == 0)
	{
		return false;
	}

	session->recvTime = timeGetTime();
	session->recvPacketCount++;

	//수신한 데이터 처리
	if (recvSize > 0)
	{
		session->RecvQ.MoveWritePos(recvSize);

		while (1)
		{
			result = CompleteRecvPacket(session);

			if (result == NONE)
				break;
			else if (result == FAIL)
			{
				_LOG(dfLOG_LEVEL_ERROR, L"PRError SessionID: %d", session->sessionID);
				return false;
			}
		}
	}

	return true;
}
bool ProcSend(DWORD sID)
{
	Session *session = g_sessionMap.find(sID)->second;
	int sendSize;
	int result;

	if (session == NULL)
		return false;

	//_LOG(dfLOG_LEVEL_DEBUG, L"ProcSend");
	//_LOG(dfLOG_LEVEL_DEBUG, L"SessionID: %d\n", sID);

	sendSize = session->SendQ.DirectDequeueSize();
	
	if (sendSize <= 0)
		return true;

	result = send(session->sock, session->SendQ.GetReadPos(), sendSize, 0);
	monitorUnit.MonitorSend();

	if (result == SOCKET_ERROR)
	{
		if (WSAGetLastError() == WSAEWOULDBLOCK)
		{
			//wouldblock 로그

			return true;
		}

		return false;
	}

	session->SendQ.MoveReadPos(result);
	return true;
}

PROCRESULT CompleteRecvPacket(Session *session)
{
	st_PACKET_HEADER header;
	int recvQSize = session->RecvQ.GetUseSize();
	Packet payload;
	BYTE endCode;
	//메시지 완성 x
	if (sizeof(header) > recvQSize)
		return NONE;

	session->RecvQ.Peek((char *)&header, sizeof(st_PACKET_HEADER));

	//패킷코드 불 일치 시 실패
	if (header.byCode != dfPACKET_CODE)
		return FAIL;

	//메시지 완성 x
	if (recvQSize < header.bySize + sizeof(header) + 1)
		return NONE;

	session->RecvQ.MoveReadPos(sizeof(st_PACKET_HEADER));

	if (session->RecvQ.Dequeue(payload, header.bySize) != header.bySize)
		return FAIL;

	//END CODE 체크
	if (session->RecvQ.Dequeue((char *)&endCode, sizeof(BYTE)) != sizeof(BYTE))
		return FAIL;
	if (endCode != dfNETWORK_PACKET_END)
		return FAIL;

	monitorUnit.MonitorRecvPacket();

	if (!PacketProc(session, header.byType, payload))
		return FAIL;

	return SUCCESS;
}

bool PacketProc(Session *session, BYTE type, Packet &p)
{
	switch (type)
	{
	case dfPACKET_CS_MOVE_START:
		MoveStart(session, p);
		break;
	case dfPACKET_CS_MOVE_STOP:
		MoveStop(session, p);
		break;
	case dfPACKET_CS_ATTACK1:
		Attack1(session, p);
		break;
	case dfPACKET_CS_ATTACK2:
		Attack2(session, p);
		break;
	case dfPACKET_CS_ATTACK3:
		Attack3(session, p);
		break;
	case dfPACKET_CS_ECHO:
		Echo(session, p);
		break;
	}

	return true;
}

bool SendUnicast(Session* session, Packet& p)
{
	if (session == NULL)
	{
		return false;
	}

	if (session->SendQ.GetFreeSize() >= p.GetDataSize())
	{
		session->SendQ.Enqueue(p);
	}
	else
	{
		_LOG(dfLOG_LEVEL_ERROR, L"SendQ size Lack");
		return false;
	}

	monitorUnit.MonitorSendPacket();

	return true;
}
bool SendBroadCast(Session* session, Packet& p, bool sendMe)
{
	
	for (auto iter = g_sessionMap.begin(); iter != g_sessionMap.end(); iter++)
	{
		if (sendMe||session->sessionID != iter->second->sessionID)
		{
			SendUnicast(iter->second, p);
		}
	}

	return true;
}

bool SendBroadCastSector(int x, int y, Packet& p, Session* exceptSession)
{
	Player *player;
	for (auto iter = g_Sector[y][x].begin(); iter != g_Sector[y][x].end(); iter++)
	{
		player = *iter;

		if (exceptSession==NULL||exceptSession->sessionID != player->sessionID)
		{
			SendUnicast(player->session, p);
		}
	}

	return true;
}
bool SendBroadCastSectorAround(Session* session, Packet& p, bool sendMe)
{
	SectorAround sectorAround;
	Player *player = g_playerMap.find(session->sessionID)->second;
	GetSectorAround(player->curSector.x, player->curSector.y, &sectorAround);

	for (int i = 0; i < sectorAround.count; i++)
	{
		if(sendMe)
			SendBroadCastSector(sectorAround.around[i].x, sectorAround.around[i].y, p);
		else
			SendBroadCastSector(sectorAround.around[i].x, sectorAround.around[i].y, p,session);
	}

	return true;
}

bool DisconnectSession(Session *session)
{
	Player *player;
	Packet p;

	if (session == NULL)
		return false;

	g_sessionMap.erase(session->sessionID);
	player = g_playerMap.find(session->sessionID)->second;

	if (player != NULL)
	{
		MakePacketRemovePlayer(p, player->sessionID);
		SectorRemovePlayer(player);
		SendBroadCastSectorAround(session, p);

		g_playerMap.erase(session->sessionID);
		delete player;
	}
	closesocket(session->sock);

	_LOG(dfLOG_LEVEL_ALWAYS, L"#Disconnect Session # SessionID : %d\n", session->sessionID);

	delete session;

	

	return true;
}