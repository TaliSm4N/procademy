#ifndef __MAKE_PACKET__
#define __MAKE_PACKET__

void MakePacketCreatePlayer(Session *session,Packet &p, DWORD id, BYTE dir, WORD x, WORD y, BYTE HP);
void MakePacketMoveStart(Session *session, Packet &p, DWORD id, BYTE dir, WORD x, WORD y);
void MakePacketMoveStop(Session *session, Packet &p, DWORD id, BYTE dir, WORD x, WORD y);
void MakePacketSync(Session *session, Packet &p, DWORD id, WORD x, WORD y);


#endif