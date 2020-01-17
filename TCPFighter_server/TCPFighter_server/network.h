#ifndef __NETWORK__
#define __NETWORK__

//접속 세션
struct Session
{
	SOCKET sock;//접속 socket
	DWORD sessionID;//session의 고유 ID
	RingBuffer RecvQ;//수신 큐
	RingBuffer SendQ;//송신 큐

	DWORD recvTime;
	DWORD recvPacketCount;
	DWORD recvSecondTick;
};

//packe process 결과
enum PROCRESULT { SUCCESS = 0, NONE, FAIL };

//네트워크 최초 실행시 listen을 열고 초기화
bool InitNetwork();

//network process
bool NetworkProcess();

//select 호출 및 처리 함수
bool CallSelect(DWORD *IDTable, SOCKET *sockTable, FD_SET *ReadSet, FD_SET *WriteSet,int cnt= FD_SETSIZE);

//연결 함수
bool ProcAccept();
bool ProcAcceptAll();

//수신 송신 함수
//sID = 세션 ID
bool ProcRecv(DWORD sID);
bool ProcSend(DWORD sID);

PROCRESULT CompleteRecvPacket(Session *session);

bool PacketProc(Session *session, BYTE type, Packet &p);


bool SendUnicast(Session* session, Packet& p);
bool SendBroadCast(Session* session, Packet& p, bool sendMe = false);
bool SendBroadCastSector(int x,int y, Packet& p, Session* exceptSession=NULL);
bool SendBroadCastSectorAround(Session* session, Packet& p, bool sendMe = false);

bool DisconnectSession(Session *session);

#endif __NETWORK__