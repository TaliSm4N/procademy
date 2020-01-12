#ifndef __PACKET_PROC__
#define __PACKET_PROC__

bool ConnectSession(Session *session);
bool MoveStart(Session *session, Packet &p);
bool MoveStop(Session *session, Packet &p);

#endif