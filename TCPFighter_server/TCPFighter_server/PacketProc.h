#ifndef __PACKET_PROC__
#define __PACKET_PROC__

bool ConnectSession(Session *session);
bool MoveStart(Session *session, Packet &p);
bool MoveStop(Session *session, Packet &p);
bool Attack1(Session *session, Packet &p);
bool Attack2(Session *session, Packet &p);
bool Attack3(Session *session, Packet &p);
Player *AttackCheck(Player *player);
bool Echo(Session *session, Packet &p);

#endif