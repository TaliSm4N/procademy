#ifndef __NETWORK__
#define __NETWORK__

//���� ����
struct Session
{
	SOCKET sock;//���� socket
	DWORD sessionID;//session�� ���� ID
	RingBuffer RecvQ;//���� ť
	RingBuffer SendQ;//�۽� ť

	DWORD recvTime;
	DWORD recvPacketCount;
	DWORD recvSecondTick;
};

//packe process ���
enum PROCRESULT { SUCCESS = 0, NONE, FAIL };

//��Ʈ��ũ ���� ����� listen�� ���� �ʱ�ȭ
bool InitNetwork();

//network process
bool NetworkProcess();

//select ȣ�� �� ó�� �Լ�
bool CallSelect(DWORD *IDTable, SOCKET *sockTable, FD_SET *ReadSet, FD_SET *WriteSet,int cnt= FD_SETSIZE);

//���� �Լ�
bool ProcAccept();
bool ProcAcceptAll();

//���� �۽� �Լ�
//sID = ���� ID
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